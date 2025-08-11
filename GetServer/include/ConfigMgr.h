
#pragma once
#include "const.h"

struct SectionInfo
{
   ~SectionInfo(){ _section_datas.clear(); }
   SectionInfo(){}

   SectionInfo(const SectionInfo &src){
      _section_datas = src._section_datas;
   }

   SectionInfo& operator = (const SectionInfo& src){
      if(&src == this){
         return *this;
      }
      this->_section_datas = src._section_datas;
      return *this;
   }

   std::map<std::string, std::string> _section_datas;
   std::string operator[](const std::string &key){
      if(_section_datas.find(key) != _section_datas.end()){
         return _section_datas[key];
      }else{
         return "";
      }
   }
};

class ConfigMgr{

   public:
      ConfigMgr(const ConfigMgr& src){
         _config_datas = src._config_datas;
      }

      ConfigMgr& operator = (const ConfigMgr& src){
         if(&src == this){
            return *this;
         }
         this->_config_datas = src._config_datas;
      }

      ~ConfigMgr(){
         _config_datas.clear();
      }

      static ConfigMgr& Instance(){
         static ConfigMgr cfg_mgr;
         return cfg_mgr;
      }
   
   SectionInfo operator[](const std::string & key){
      if(_config_datas.find(key) != _config_datas.end()){
         return _config_datas[key];
      }else{
         return SectionInfo();
      }
   }
   private:
      ConfigMgr();
      std::map<std::string, SectionInfo> _config_datas;
};
