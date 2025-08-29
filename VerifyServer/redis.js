const config_module = require("./config")
const redis = require("ioredis")

const RedisCil = new redis({
   host : config_module.redis_host,
   port : config_module.redis_port,
   password : config_module.redis_passwd
});

RedisCil.on("error", function(err){
   console.log("Redis Connect error")
   RedisCil.quit()
});

async function GetRedis(key){
   try{

      const result = await RedisCil.get(key)
      if(result == null){
         console.log("result : ", "<" + result + ">", "this key cannot find ...")
         return null;
      }
      console.log("result : ", "<" + result + ">", "this key find success ...")
      return result
   }catch(error){
      console.log("redis error is ", error)
      return null
   }
}

async function QueryRedis(key){
   try{

      const result = await RedisCil.exists(key)
      if(result == 0){
         console.log("result : ", "<" + result + ">", "this key is null ...")
         return null;
      }
      console.log("result : ", "<" + result + ">", "with this value ...")
      return result
   }catch(error){
      console.log("query error is ", error)
      return null
   }
}

async function SetRedisExpire(key, value, exptime){
   try{

      await RedisCil.set(key, value)
      await RedisCil.expire(key, exptime)
      return true
   }catch(error){
      console.log("setredisexpire error is ", error)
      return false
   }
}

function Quit(){
   RedisCil.quit()
   console.log("redis quit")
}

module.exports = { GetRedis, QueryRedis, Quit, SetRedisExpire}



