#include <iostream>
#include <hiredis/hiredis.h>
#include <chrono>
#include <vector>

class RedisClient {
public:
    RedisClient(const std::string& host, int port, const std::string& password) 
        : m_host(host), m_port(port), m_password(password) {}
    
    bool connect() {
        // 带超时的连接（1.5秒）
        struct timeval timeout = {1, 500000};
        m_conn = redisConnectWithTimeout(m_host.c_str(), m_port, timeout);
        
        if (!m_conn || m_conn->err) {
            std::cerr << "Connection error: " 
                     << (m_conn ? m_conn->errstr : "Can't allocate context") 
                     << std::endl;
            return false;
        }

        // 认证
        auto reply = command("AUTH %s", m_password.c_str());
        if (!reply || reply->type == REDIS_REPLY_ERROR) {
            std::cerr << "Auth failed: " 
                     << (reply ? reply->str : "NULL reply") 
                     << std::endl;
            return false;
        }
        freeReplyObject(reply);
        return true;
    }

    redisReply* command(const char* format, ...) {
        va_list ap;
        va_start(ap, format);
        redisReply* r = (redisReply*)redisvCommand(m_conn, format, ap);
        va_end(ap);
        return r;
    }

    ~RedisClient() {
        if (m_conn) redisFree(m_conn);
    }

    void testAll() {
        testString();
        testHash();
        testList();
        testSet();
        testPerformance();
    }

private:
    void testString() {
        std::cout << "\n=== Testing String ===" << std::endl;
        
        // SET/GET
        auto reply = command("SET str_key hello_redis");
        checkReply(reply, "SET str_key");
        freeReplyObject(reply);
        
        reply = command("GET str_key");
        if (checkReply(reply, "GET str_key")) {
            std::cout << "GET str_key: " << reply->str << std::endl;
        }
        freeReplyObject(reply);
        
        // INCR
        reply = command("INCR counter");
        checkReply(reply, "INCR counter");
        freeReplyObject(reply);
    }

    void testHash() {
        std::cout << "\n=== Testing Hash ===" << std::endl;
        
        auto reply = command("HSET user:1000 name Alice age 30");
        checkReply(reply, "HSET user:1000");
        freeReplyObject(reply);
        
        reply = command("HGETALL user:1000");
        if (checkReply(reply, "HGETALL user:1000") && reply->type == REDIS_REPLY_ARRAY) {
            for (size_t i = 0; i < reply->elements; i += 2) {
                std::cout << reply->element[i]->str << ": " 
                         << reply->element[i+1]->str << std::endl;
            }
        }
        freeReplyObject(reply);
    }

    void testList() {
        std::cout << "\n=== Testing List ===" << std::endl;
        
        auto reply = command("RPUSH mylist A B C D");
        checkReply(reply, "RPUSH mylist");
        freeReplyObject(reply);
        
        reply = command("LRANGE mylist 0 -1");
        if (checkReply(reply, "LRANGE mylist") && reply->type == REDIS_REPLY_ARRAY) {
            std::cout << "List items: ";
            for (size_t i = 0; i < reply->elements; i++) {
                std::cout << reply->element[i]->str << " ";
            }
            std::cout << std::endl;
        }
        freeReplyObject(reply);
    }

    void testSet() {
        std::cout << "\n=== Testing Set ===" << std::endl;
        
        auto reply = command("SADD myset 1 2 3 4 5");
        checkReply(reply, "SADD myset");
        freeReplyObject(reply);
        
        reply = command("SMEMBERS myset");
        if (checkReply(reply, "SMEMBERS myset") && reply->type == REDIS_REPLY_ARRAY) {
            std::cout << "Set members: ";
            for (size_t i = 0; i < reply->elements; i++) {
                std::cout << reply->element[i]->str << " ";
            }
            std::cout << std::endl;
        }
        freeReplyObject(reply);
    }

    void testPerformance() {
        std::cout << "\n=== Testing Performance ===" << std::endl;
        
        const int TOTAL_OPS = 10000;
        auto start = std::chrono::high_resolution_clock::now();
        
        for (int i = 0; i < TOTAL_OPS; ++i) {
            auto reply = command("SET perf_key_%d value_%d", i, i);
            freeReplyObject(reply);
        }
        
        auto end = std::chrono::high_resolution_clock::now();
        auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count();
        
        std::cout << "Completed " << TOTAL_OPS << " SET operations in "
                  << duration << " ms" << std::endl;
        std::cout << "Average: " << (TOTAL_OPS * 1000.0 / duration) 
                  << " ops/sec" << std::endl;
    }

    bool checkReply(redisReply* reply, const std::string& cmd) {
        if (!reply) {
            std::cerr << cmd << " failed (NULL reply)" << std::endl;
            return false;
        }
        if (reply->type == REDIS_REPLY_ERROR) {
            std::cerr << cmd << " failed: " << reply->str << std::endl;
            return false;
        }
        return true;
    }

    redisContext* m_conn = nullptr;
    std::string m_host;
    int m_port;
    std::string m_password;
};

int main() {
    // 从环境变量获取密码，默认123456
    const char* password = std::getenv("REDIS_PASSWORD");
    if (!password) password = "123456";

    RedisClient redis("127.0.0.1", 6379, password);
    
    if (!redis.connect()) {
        std::cerr << "Failed to connect to Redis" << std::endl;
        return 1;
    }

    std::cout << "Redis client connected successfully!" << std::endl;
    redis.testAll();

    return 0;
}