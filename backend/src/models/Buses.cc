

#include "Buses.h"
#include <drogon/utils/Utilities.h>
#include <string>

using namespace drogon;
using namespace drogon::orm;
using namespace drogon_model::postgres;

const std::string Buses::Cols::_id = "\"id\"";
const std::string Buses::Cols::_license_plate = "\"license_plate\"";
const std::string Buses::Cols::_capacity = "\"capacity\"";
const std::string Buses::Cols::_status = "\"status\"";
const std::string Buses::Cols::_created_at = "\"created_at\"";
const std::string Buses::primaryKeyName = "id";
const bool Buses::hasPrimaryKey = true;
const std::string Buses::tableName = "\"buses\"";

const std::vector<typename Buses::MetaData> Buses::metaData_={
{"id","std::string","uuid",0,0,1,1},
{"license_plate","std::string","character varying",20,0,0,1},
{"capacity","int32_t","integer",4,0,0,1},
{"status","std::string","character varying",50,0,0,0},
{"created_at","::trantor::Date","timestamp without time zone",0,0,0,0}
};
const std::string &Buses::getColumnName(size_t index) noexcept(false)
{
    assert(index < metaData_.size());
    return metaData_[index].colName_;
}
Buses::Buses(const Row &r, const ssize_t indexOffset) noexcept
{
    if(indexOffset < 0)
    {
        if(!r["id"].isNull())
        {
            id_=std::make_shared<std::string>(r["id"].as<std::string>());
        }
        if(!r["license_plate"].isNull())
        {
            licensePlate_=std::make_shared<std::string>(r["license_plate"].as<std::string>());
        }
        if(!r["capacity"].isNull())
        {
            capacity_=std::make_shared<int32_t>(r["capacity"].as<int32_t>());
        }
        if(!r["status"].isNull())
        {
            status_=std::make_shared<std::string>(r["status"].as<std::string>());
        }
        if(!r["created_at"].isNull())
        {
            auto timeStr = r["created_at"].as<std::string>();
            struct tm stm;
            memset(&stm,0,sizeof(stm));
            auto p = strptime(timeStr.c_str(),"%Y-%m-%d %H:%M:%S",&stm);
            time_t t = mktime(&stm);
            size_t decimalNum = 0;
            if(p)
            {
                if(*p=='.')
                {
                    std::string decimals(p+1,&timeStr[timeStr.length()]);
                    while(decimals.length()<6)
                    {
                        decimals += "0";
                    }
                    decimalNum = (size_t)atol(decimals.c_str());
                }
                createdAt_=std::make_shared<::trantor::Date>(t*1000000+decimalNum);
            }
        }
    }
    else
    {
        size_t offset = (size_t)indexOffset;
        if(offset + 5 > r.size())
        {
            LOG_FATAL << "Invalid SQL result for this model";
            return;
        }
        size_t index;
        index = offset + 0;
        if(!r[index].isNull())
        {
            id_=std::make_shared<std::string>(r[index].as<std::string>());
        }
        index = offset + 1;
        if(!r[index].isNull())
        {
            licensePlate_=std::make_shared<std::string>(r[index].as<std::string>());
        }
        index = offset + 2;
        if(!r[index].isNull())
        {
            capacity_=std::make_shared<int32_t>(r[index].as<int32_t>());
        }
        index = offset + 3;
        if(!r[index].isNull())
        {
            status_=std::make_shared<std::string>(r[index].as<std::string>());
        }
        index = offset + 4;
        if(!r[index].isNull())
        {
            auto timeStr = r[index].as<std::string>();
            struct tm stm;
            memset(&stm,0,sizeof(stm));
            auto p = strptime(timeStr.c_str(),"%Y-%m-%d %H:%M:%S",&stm);
            time_t t = mktime(&stm);
            size_t decimalNum = 0;
            if(p)
            {
                if(*p=='.')
                {
                    std::string decimals(p+1,&timeStr[timeStr.length()]);
                    while(decimals.length()<6)
                    {
                        decimals += "0";
                    }
                    decimalNum = (size_t)atol(decimals.c_str());
                }
                createdAt_=std::make_shared<::trantor::Date>(t*1000000+decimalNum);
            }
        }
    }

}

Buses::Buses(const Json::Value &pJson, const std::vector<std::string> &pMasqueradingVector) noexcept(false)
{
    if(pMasqueradingVector.size() != 5)
    {
        LOG_ERROR << "Bad masquerading vector";
        return;
    }
    if(!pMasqueradingVector[0].empty() && pJson.isMember(pMasqueradingVector[0]))
    {
        dirtyFlag_[0] = true;
        if(!pJson[pMasqueradingVector[0]].isNull())
        {
            id_=std::make_shared<std::string>(pJson[pMasqueradingVector[0]].asString());
        }
    }
    if(!pMasqueradingVector[1].empty() && pJson.isMember(pMasqueradingVector[1]))
    {
        dirtyFlag_[1] = true;
        if(!pJson[pMasqueradingVector[1]].isNull())
        {
            licensePlate_=std::make_shared<std::string>(pJson[pMasqueradingVector[1]].asString());
        }
    }
    if(!pMasqueradingVector[2].empty() && pJson.isMember(pMasqueradingVector[2]))
    {
        dirtyFlag_[2] = true;
        if(!pJson[pMasqueradingVector[2]].isNull())
        {
            capacity_=std::make_shared<int32_t>((int32_t)pJson[pMasqueradingVector[2]].asInt64());
        }
    }
    if(!pMasqueradingVector[3].empty() && pJson.isMember(pMasqueradingVector[3]))
    {
        dirtyFlag_[3] = true;
        if(!pJson[pMasqueradingVector[3]].isNull())
        {
            status_=std::make_shared<std::string>(pJson[pMasqueradingVector[3]].asString());
        }
    }
    if(!pMasqueradingVector[4].empty() && pJson.isMember(pMasqueradingVector[4]))
    {
        dirtyFlag_[4] = true;
        if(!pJson[pMasqueradingVector[4]].isNull())
        {
            auto timeStr = pJson[pMasqueradingVector[4]].asString();
            struct tm stm;
            memset(&stm,0,sizeof(stm));
            auto p = strptime(timeStr.c_str(),"%Y-%m-%d %H:%M:%S",&stm);
            time_t t = mktime(&stm);
            size_t decimalNum = 0;
            if(p)
            {
                if(*p=='.')
                {
                    std::string decimals(p+1,&timeStr[timeStr.length()]);
                    while(decimals.length()<6)
                    {
                        decimals += "0";
                    }
                    decimalNum = (size_t)atol(decimals.c_str());
                }
                createdAt_=std::make_shared<::trantor::Date>(t*1000000+decimalNum);
            }
        }
    }
}

Buses::Buses(const Json::Value &pJson) noexcept(false)
{
    if(pJson.isMember("id"))
    {
        dirtyFlag_[0]=true;
        if(!pJson["id"].isNull())
        {
            id_=std::make_shared<std::string>(pJson["id"].asString());
        }
    }
    if(pJson.isMember("license_plate"))
    {
        dirtyFlag_[1]=true;
        if(!pJson["license_plate"].isNull())
        {
            licensePlate_=std::make_shared<std::string>(pJson["license_plate"].asString());
        }
    }
    if(pJson.isMember("capacity"))
    {
        dirtyFlag_[2]=true;
        if(!pJson["capacity"].isNull())
        {
            capacity_=std::make_shared<int32_t>((int32_t)pJson["capacity"].asInt64());
        }
    }
    if(pJson.isMember("status"))
    {
        dirtyFlag_[3]=true;
        if(!pJson["status"].isNull())
        {
            status_=std::make_shared<std::string>(pJson["status"].asString());
        }
    }
    if(pJson.isMember("created_at"))
    {
        dirtyFlag_[4]=true;
        if(!pJson["created_at"].isNull())
        {
            auto timeStr = pJson["created_at"].asString();
            struct tm stm;
            memset(&stm,0,sizeof(stm));
            auto p = strptime(timeStr.c_str(),"%Y-%m-%d %H:%M:%S",&stm);
            time_t t = mktime(&stm);
            size_t decimalNum = 0;
            if(p)
            {
                if(*p=='.')
                {
                    std::string decimals(p+1,&timeStr[timeStr.length()]);
                    while(decimals.length()<6)
                    {
                        decimals += "0";
                    }
                    decimalNum = (size_t)atol(decimals.c_str());
                }
                createdAt_=std::make_shared<::trantor::Date>(t*1000000+decimalNum);
            }
        }
    }
}

void Buses::updateByMasqueradedJson(const Json::Value &pJson,
                                            const std::vector<std::string> &pMasqueradingVector) noexcept(false)
{
    if(pMasqueradingVector.size() != 5)
    {
        LOG_ERROR << "Bad masquerading vector";
        return;
    }
    if(!pMasqueradingVector[0].empty() && pJson.isMember(pMasqueradingVector[0]))
    {
        if(!pJson[pMasqueradingVector[0]].isNull())
        {
            id_=std::make_shared<std::string>(pJson[pMasqueradingVector[0]].asString());
        }
    }
    if(!pMasqueradingVector[1].empty() && pJson.isMember(pMasqueradingVector[1]))
    {
        dirtyFlag_[1] = true;
        if(!pJson[pMasqueradingVector[1]].isNull())
        {
            licensePlate_=std::make_shared<std::string>(pJson[pMasqueradingVector[1]].asString());
        }
    }
    if(!pMasqueradingVector[2].empty() && pJson.isMember(pMasqueradingVector[2]))
    {
        dirtyFlag_[2] = true;
        if(!pJson[pMasqueradingVector[2]].isNull())
        {
            capacity_=std::make_shared<int32_t>((int32_t)pJson[pMasqueradingVector[2]].asInt64());
        }
    }
    if(!pMasqueradingVector[3].empty() && pJson.isMember(pMasqueradingVector[3]))
    {
        dirtyFlag_[3] = true;
        if(!pJson[pMasqueradingVector[3]].isNull())
        {
            status_=std::make_shared<std::string>(pJson[pMasqueradingVector[3]].asString());
        }
    }
    if(!pMasqueradingVector[4].empty() && pJson.isMember(pMasqueradingVector[4]))
    {
        dirtyFlag_[4] = true;
        if(!pJson[pMasqueradingVector[4]].isNull())
        {
            auto timeStr = pJson[pMasqueradingVector[4]].asString();
            struct tm stm;
            memset(&stm,0,sizeof(stm));
            auto p = strptime(timeStr.c_str(),"%Y-%m-%d %H:%M:%S",&stm);
            time_t t = mktime(&stm);
            size_t decimalNum = 0;
            if(p)
            {
                if(*p=='.')
                {
                    std::string decimals(p+1,&timeStr[timeStr.length()]);
                    while(decimals.length()<6)
                    {
                        decimals += "0";
                    }
                    decimalNum = (size_t)atol(decimals.c_str());
                }
                createdAt_=std::make_shared<::trantor::Date>(t*1000000+decimalNum);
            }
        }
    }
}

void Buses::updateByJson(const Json::Value &pJson) noexcept(false)
{
    if(pJson.isMember("id"))
    {
        if(!pJson["id"].isNull())
        {
            id_=std::make_shared<std::string>(pJson["id"].asString());
        }
    }
    if(pJson.isMember("license_plate"))
    {
        dirtyFlag_[1] = true;
        if(!pJson["license_plate"].isNull())
        {
            licensePlate_=std::make_shared<std::string>(pJson["license_plate"].asString());
        }
    }
    if(pJson.isMember("capacity"))
    {
        dirtyFlag_[2] = true;
        if(!pJson["capacity"].isNull())
        {
            capacity_=std::make_shared<int32_t>((int32_t)pJson["capacity"].asInt64());
        }
    }
    if(pJson.isMember("status"))
    {
        dirtyFlag_[3] = true;
        if(!pJson["status"].isNull())
        {
            status_=std::make_shared<std::string>(pJson["status"].asString());
        }
    }
    if(pJson.isMember("created_at"))
    {
        dirtyFlag_[4] = true;
        if(!pJson["created_at"].isNull())
        {
            auto timeStr = pJson["created_at"].asString();
            struct tm stm;
            memset(&stm,0,sizeof(stm));
            auto p = strptime(timeStr.c_str(),"%Y-%m-%d %H:%M:%S",&stm);
            time_t t = mktime(&stm);
            size_t decimalNum = 0;
            if(p)
            {
                if(*p=='.')
                {
                    std::string decimals(p+1,&timeStr[timeStr.length()]);
                    while(decimals.length()<6)
                    {
                        decimals += "0";
                    }
                    decimalNum = (size_t)atol(decimals.c_str());
                }
                createdAt_=std::make_shared<::trantor::Date>(t*1000000+decimalNum);
            }
        }
    }
}

const std::string &Buses::getValueOfId() const noexcept
{
    static const std::string defaultValue = std::string();
    if(id_)
        return *id_;
    return defaultValue;
}
const std::shared_ptr<std::string> &Buses::getId() const noexcept
{
    return id_;
}
void Buses::setId(const std::string &pId) noexcept
{
    id_ = std::make_shared<std::string>(pId);
    dirtyFlag_[0] = true;
}
void Buses::setId(std::string &&pId) noexcept
{
    id_ = std::make_shared<std::string>(std::move(pId));
    dirtyFlag_[0] = true;
}
const typename Buses::PrimaryKeyType & Buses::getPrimaryKey() const
{
    assert(id_);
    return *id_;
}

const std::string &Buses::getValueOfLicensePlate() const noexcept
{
    static const std::string defaultValue = std::string();
    if(licensePlate_)
        return *licensePlate_;
    return defaultValue;
}
const std::shared_ptr<std::string> &Buses::getLicensePlate() const noexcept
{
    return licensePlate_;
}
void Buses::setLicensePlate(const std::string &pLicensePlate) noexcept
{
    licensePlate_ = std::make_shared<std::string>(pLicensePlate);
    dirtyFlag_[1] = true;
}
void Buses::setLicensePlate(std::string &&pLicensePlate) noexcept
{
    licensePlate_ = std::make_shared<std::string>(std::move(pLicensePlate));
    dirtyFlag_[1] = true;
}

const int32_t &Buses::getValueOfCapacity() const noexcept
{
    static const int32_t defaultValue = int32_t();
    if(capacity_)
        return *capacity_;
    return defaultValue;
}
const std::shared_ptr<int32_t> &Buses::getCapacity() const noexcept
{
    return capacity_;
}
void Buses::setCapacity(const int32_t &pCapacity) noexcept
{
    capacity_ = std::make_shared<int32_t>(pCapacity);
    dirtyFlag_[2] = true;
}

const std::string &Buses::getValueOfStatus() const noexcept
{
    static const std::string defaultValue = std::string();
    if(status_)
        return *status_;
    return defaultValue;
}
const std::shared_ptr<std::string> &Buses::getStatus() const noexcept
{
    return status_;
}
void Buses::setStatus(const std::string &pStatus) noexcept
{
    status_ = std::make_shared<std::string>(pStatus);
    dirtyFlag_[3] = true;
}
void Buses::setStatus(std::string &&pStatus) noexcept
{
    status_ = std::make_shared<std::string>(std::move(pStatus));
    dirtyFlag_[3] = true;
}
void Buses::setStatusToNull() noexcept
{
    status_.reset();
    dirtyFlag_[3] = true;
}

const ::trantor::Date &Buses::getValueOfCreatedAt() const noexcept
{
    static const ::trantor::Date defaultValue = ::trantor::Date();
    if(createdAt_)
        return *createdAt_;
    return defaultValue;
}
const std::shared_ptr<::trantor::Date> &Buses::getCreatedAt() const noexcept
{
    return createdAt_;
}
void Buses::setCreatedAt(const ::trantor::Date &pCreatedAt) noexcept
{
    createdAt_ = std::make_shared<::trantor::Date>(pCreatedAt);
    dirtyFlag_[4] = true;
}
void Buses::setCreatedAtToNull() noexcept
{
    createdAt_.reset();
    dirtyFlag_[4] = true;
}

void Buses::updateId(const uint64_t id)
{
}

const std::vector<std::string> &Buses::insertColumns() noexcept
{
    static const std::vector<std::string> inCols={
        "id",
        "license_plate",
        "capacity",
        "status",
        "created_at"
    };
    return inCols;
}

void Buses::outputArgs(drogon::orm::internal::SqlBinder &binder) const
{
    if(dirtyFlag_[0])
    {
        if(getId())
        {
            binder << getValueOfId();
        }
        else
        {
            binder << nullptr;
        }
    }
    if(dirtyFlag_[1])
    {
        if(getLicensePlate())
        {
            binder << getValueOfLicensePlate();
        }
        else
        {
            binder << nullptr;
        }
    }
    if(dirtyFlag_[2])
    {
        if(getCapacity())
        {
            binder << getValueOfCapacity();
        }
        else
        {
            binder << nullptr;
        }
    }
    if(dirtyFlag_[3])
    {
        if(getStatus())
        {
            binder << getValueOfStatus();
        }
        else
        {
            binder << nullptr;
        }
    }
    if(dirtyFlag_[4])
    {
        if(getCreatedAt())
        {
            binder << getValueOfCreatedAt();
        }
        else
        {
            binder << nullptr;
        }
    }
}

const std::vector<std::string> Buses::updateColumns() const
{
    std::vector<std::string> ret;
    if(dirtyFlag_[0])
    {
        ret.push_back(getColumnName(0));
    }
    if(dirtyFlag_[1])
    {
        ret.push_back(getColumnName(1));
    }
    if(dirtyFlag_[2])
    {
        ret.push_back(getColumnName(2));
    }
    if(dirtyFlag_[3])
    {
        ret.push_back(getColumnName(3));
    }
    if(dirtyFlag_[4])
    {
        ret.push_back(getColumnName(4));
    }
    return ret;
}

void Buses::updateArgs(drogon::orm::internal::SqlBinder &binder) const
{
    if(dirtyFlag_[0])
    {
        if(getId())
        {
            binder << getValueOfId();
        }
        else
        {
            binder << nullptr;
        }
    }
    if(dirtyFlag_[1])
    {
        if(getLicensePlate())
        {
            binder << getValueOfLicensePlate();
        }
        else
        {
            binder << nullptr;
        }
    }
    if(dirtyFlag_[2])
    {
        if(getCapacity())
        {
            binder << getValueOfCapacity();
        }
        else
        {
            binder << nullptr;
        }
    }
    if(dirtyFlag_[3])
    {
        if(getStatus())
        {
            binder << getValueOfStatus();
        }
        else
        {
            binder << nullptr;
        }
    }
    if(dirtyFlag_[4])
    {
        if(getCreatedAt())
        {
            binder << getValueOfCreatedAt();
        }
        else
        {
            binder << nullptr;
        }
    }
}
Json::Value Buses::toJson() const
{
    Json::Value ret;
    if(getId())
    {
        ret["id"]=getValueOfId();
    }
    else
    {
        ret["id"]=Json::Value();
    }
    if(getLicensePlate())
    {
        ret["license_plate"]=getValueOfLicensePlate();
    }
    else
    {
        ret["license_plate"]=Json::Value();
    }
    if(getCapacity())
    {
        ret["capacity"]=getValueOfCapacity();
    }
    else
    {
        ret["capacity"]=Json::Value();
    }
    if(getStatus())
    {
        ret["status"]=getValueOfStatus();
    }
    else
    {
        ret["status"]=Json::Value();
    }
    if(getCreatedAt())
    {
        ret["created_at"]=getCreatedAt()->toDbStringLocal();
    }
    else
    {
        ret["created_at"]=Json::Value();
    }
    return ret;
}

std::string Buses::toString() const
{
    return toJson().toStyledString();
}

Json::Value Buses::toMasqueradedJson(
    const std::vector<std::string> &pMasqueradingVector) const
{
    Json::Value ret;
    if(pMasqueradingVector.size() == 5)
    {
        if(!pMasqueradingVector[0].empty())
        {
            if(getId())
            {
                ret[pMasqueradingVector[0]]=getValueOfId();
            }
            else
            {
                ret[pMasqueradingVector[0]]=Json::Value();
            }
        }
        if(!pMasqueradingVector[1].empty())
        {
            if(getLicensePlate())
            {
                ret[pMasqueradingVector[1]]=getValueOfLicensePlate();
            }
            else
            {
                ret[pMasqueradingVector[1]]=Json::Value();
            }
        }
        if(!pMasqueradingVector[2].empty())
        {
            if(getCapacity())
            {
                ret[pMasqueradingVector[2]]=getValueOfCapacity();
            }
            else
            {
                ret[pMasqueradingVector[2]]=Json::Value();
            }
        }
        if(!pMasqueradingVector[3].empty())
        {
            if(getStatus())
            {
                ret[pMasqueradingVector[3]]=getValueOfStatus();
            }
            else
            {
                ret[pMasqueradingVector[3]]=Json::Value();
            }
        }
        if(!pMasqueradingVector[4].empty())
        {
            if(getCreatedAt())
            {
                ret[pMasqueradingVector[4]]=getCreatedAt()->toDbStringLocal();
            }
            else
            {
                ret[pMasqueradingVector[4]]=Json::Value();
            }
        }
        return ret;
    }
    LOG_ERROR << "Masquerade failed";
    if(getId())
    {
        ret["id"]=getValueOfId();
    }
    else
    {
        ret["id"]=Json::Value();
    }
    if(getLicensePlate())
    {
        ret["license_plate"]=getValueOfLicensePlate();
    }
    else
    {
        ret["license_plate"]=Json::Value();
    }
    if(getCapacity())
    {
        ret["capacity"]=getValueOfCapacity();
    }
    else
    {
        ret["capacity"]=Json::Value();
    }
    if(getStatus())
    {
        ret["status"]=getValueOfStatus();
    }
    else
    {
        ret["status"]=Json::Value();
    }
    if(getCreatedAt())
    {
        ret["created_at"]=getCreatedAt()->toDbStringLocal();
    }
    else
    {
        ret["created_at"]=Json::Value();
    }
    return ret;
}

bool Buses::validateJsonForCreation(const Json::Value &pJson, std::string &err)
{
    if(pJson.isMember("id"))
    {
        if(!validJsonOfField(0, "id", pJson["id"], err, true))
            return false;
    }
    if(pJson.isMember("license_plate"))
    {
        if(!validJsonOfField(1, "license_plate", pJson["license_plate"], err, true))
            return false;
    }
    else
    {
        err="The license_plate column cannot be null";
        return false;
    }
    if(pJson.isMember("capacity"))
    {
        if(!validJsonOfField(2, "capacity", pJson["capacity"], err, true))
            return false;
    }
    else
    {
        err="The capacity column cannot be null";
        return false;
    }
    if(pJson.isMember("status"))
    {
        if(!validJsonOfField(3, "status", pJson["status"], err, true))
            return false;
    }
    if(pJson.isMember("created_at"))
    {
        if(!validJsonOfField(4, "created_at", pJson["created_at"], err, true))
            return false;
    }
    return true;
}
bool Buses::validateMasqueradedJsonForCreation(const Json::Value &pJson,
                                               const std::vector<std::string> &pMasqueradingVector,
                                               std::string &err)
{
    if(pMasqueradingVector.size() != 5)
    {
        err = "Bad masquerading vector";
        return false;
    }
    try {
      if(!pMasqueradingVector[0].empty())
      {
          if(pJson.isMember(pMasqueradingVector[0]))
          {
              if(!validJsonOfField(0, pMasqueradingVector[0], pJson[pMasqueradingVector[0]], err, true))
                  return false;
          }
      }
      if(!pMasqueradingVector[1].empty())
      {
          if(pJson.isMember(pMasqueradingVector[1]))
          {
              if(!validJsonOfField(1, pMasqueradingVector[1], pJson[pMasqueradingVector[1]], err, true))
                  return false;
          }
        else
        {
            err="The " + pMasqueradingVector[1] + " column cannot be null";
            return false;
        }
      }
      if(!pMasqueradingVector[2].empty())
      {
          if(pJson.isMember(pMasqueradingVector[2]))
          {
              if(!validJsonOfField(2, pMasqueradingVector[2], pJson[pMasqueradingVector[2]], err, true))
                  return false;
          }
        else
        {
            err="The " + pMasqueradingVector[2] + " column cannot be null";
            return false;
        }
      }
      if(!pMasqueradingVector[3].empty())
      {
          if(pJson.isMember(pMasqueradingVector[3]))
          {
              if(!validJsonOfField(3, pMasqueradingVector[3], pJson[pMasqueradingVector[3]], err, true))
                  return false;
          }
      }
      if(!pMasqueradingVector[4].empty())
      {
          if(pJson.isMember(pMasqueradingVector[4]))
          {
              if(!validJsonOfField(4, pMasqueradingVector[4], pJson[pMasqueradingVector[4]], err, true))
                  return false;
          }
      }
    }
    catch(const Json::LogicError &e)
    {
      err = e.what();
      return false;
    }
    return true;
}
bool Buses::validateJsonForUpdate(const Json::Value &pJson, std::string &err)
{
    if(pJson.isMember("id"))
    {
        if(!validJsonOfField(0, "id", pJson["id"], err, false))
            return false;
    }
    else
    {
        err = "The value of primary key must be set in the json object for update";
        return false;
    }
    if(pJson.isMember("license_plate"))
    {
        if(!validJsonOfField(1, "license_plate", pJson["license_plate"], err, false))
            return false;
    }
    if(pJson.isMember("capacity"))
    {
        if(!validJsonOfField(2, "capacity", pJson["capacity"], err, false))
            return false;
    }
    if(pJson.isMember("status"))
    {
        if(!validJsonOfField(3, "status", pJson["status"], err, false))
            return false;
    }
    if(pJson.isMember("created_at"))
    {
        if(!validJsonOfField(4, "created_at", pJson["created_at"], err, false))
            return false;
    }
    return true;
}
bool Buses::validateMasqueradedJsonForUpdate(const Json::Value &pJson,
                                             const std::vector<std::string> &pMasqueradingVector,
                                             std::string &err)
{
    if(pMasqueradingVector.size() != 5)
    {
        err = "Bad masquerading vector";
        return false;
    }
    try {
      if(!pMasqueradingVector[0].empty() && pJson.isMember(pMasqueradingVector[0]))
      {
          if(!validJsonOfField(0, pMasqueradingVector[0], pJson[pMasqueradingVector[0]], err, false))
              return false;
      }
    else
    {
        err = "The value of primary key must be set in the json object for update";
        return false;
    }
      if(!pMasqueradingVector[1].empty() && pJson.isMember(pMasqueradingVector[1]))
      {
          if(!validJsonOfField(1, pMasqueradingVector[1], pJson[pMasqueradingVector[1]], err, false))
              return false;
      }
      if(!pMasqueradingVector[2].empty() && pJson.isMember(pMasqueradingVector[2]))
      {
          if(!validJsonOfField(2, pMasqueradingVector[2], pJson[pMasqueradingVector[2]], err, false))
              return false;
      }
      if(!pMasqueradingVector[3].empty() && pJson.isMember(pMasqueradingVector[3]))
      {
          if(!validJsonOfField(3, pMasqueradingVector[3], pJson[pMasqueradingVector[3]], err, false))
              return false;
      }
      if(!pMasqueradingVector[4].empty() && pJson.isMember(pMasqueradingVector[4]))
      {
          if(!validJsonOfField(4, pMasqueradingVector[4], pJson[pMasqueradingVector[4]], err, false))
              return false;
      }
    }
    catch(const Json::LogicError &e)
    {
      err = e.what();
      return false;
    }
    return true;
}
bool Buses::validJsonOfField(size_t index,
                             const std::string &fieldName,
                             const Json::Value &pJson,
                             std::string &err,
                             bool isForCreation)
{
    switch(index)
    {
        case 0:
            if(pJson.isNull())
            {
                err="The " + fieldName + " column cannot be null";
                return false;
            }
            if(!pJson.isString())
            {
                err="Type error in the "+fieldName+" field";
                return false;
            }
            break;
        case 1:
            if(pJson.isNull())
            {
                err="The " + fieldName + " column cannot be null";
                return false;
            }
            if(!pJson.isString())
            {
                err="Type error in the "+fieldName+" field";
                return false;
            }
            if(pJson.isString() && std::wstring_convert<std::codecvt_utf8_utf16<wchar_t>, wchar_t>{}
                .from_bytes(pJson.asCString()).size() > 20)
            {
                err="String length exceeds limit for the " +
                    fieldName +
                    " field (the maximum value is 20)";
                return false;
            }
            break;
        case 2:
            if(pJson.isNull())
            {
                err="The " + fieldName + " column cannot be null";
                return false;
            }
            if(!pJson.isInt())
            {
                err="Type error in the "+fieldName+" field";
                return false;
            }
            break;
        case 3:
            if(pJson.isNull())
            {
                return true;
            }
            if(!pJson.isString())
            {
                err="Type error in the "+fieldName+" field";
                return false;
            }
            if(pJson.isString() && std::wstring_convert<std::codecvt_utf8_utf16<wchar_t>, wchar_t>{}
                .from_bytes(pJson.asCString()).size() > 50)
            {
                err="String length exceeds limit for the " +
                    fieldName +
                    " field (the maximum value is 50)";
                return false;
            }
            break;
        case 4:
            if(pJson.isNull())
            {
                return true;
            }
            if(!pJson.isString())
            {
                err="Type error in the "+fieldName+" field";
                return false;
            }
            break;
        default:
            err="Internal error in the server";
            return false;
    }
    return true;
}
