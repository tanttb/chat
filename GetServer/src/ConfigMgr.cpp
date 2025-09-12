
#include "ConfigMgr.h"

ConfigMgr::ConfigMgr(){

   boost::filesystem::path cur_path = boost::filesystem::current_path();
   boost::filesystem::path config_path = cur_path / "../../config.ini";
   std::cout << "Config path: " << config_path << std::endl;

   boost::property_tree::ptree pt;
   boost::property_tree::read_ini(config_path.string(), pt);
   
   for(const auto & section_pair : pt){
      const std::string& sec_na = section_pair.first;
      const boost::property_tree::ptree& sec_tree = section_pair.second;
      
      std::map<std::string, std::string> sec_config;
      for(const auto &key_value : sec_tree){
         sec_config.insert({key_value.first, key_value.second.get_value<std::string>()});
      }

      SectionInfo secinfo;
      secinfo._section_datas = sec_config;
      _config_datas[sec_na] = secinfo;
   }

   // std::string gate_p = _config_datas["GateServer"]["Port"];
   // std::cout << gate_p.c_str() << std::endl;

}