

#include "Stops.h"
#include <drogon/utils/Utilities.h>
#include <string>

using namespace drogon;
using namespace drogon::orm;
using namespace drogon_model::postgres;

const std::string Stops::Cols::_id = "\"id\"";
const std::string Stops::Cols::_route_id = "\"route_id\"";
const std::string Stops::Cols::_name = "\"name\"";
const std::string Stops::Cols::_latitude = "\"latitude\"";
const std::string Stops::Cols::_longitude = "\"longitude\"";
const std::string Stops::Cols::_sequence_order = "\"sequence_order\"";
const std::string Stops::primaryKeyName = "id";
const bool Stops::hasPrimaryKey = true;
const std::string Stops::tableName = "\"stops\"";

const std::vector<typename Stops::MetaData> Stops::metaData_={
{"id","std::string","uuid",0,0,1,1},
{"route_id","std::string","uuid",0,0,0,0},
{"name","std::string","character varying",100,0,0,1},
{"latitude","std::string","numeric",0,0,0,1},
{"longitude","std::string","numeric",0,0,0,1},
{"sequence_order","int32_t","integer",4,0,0,1}
};
const std::string &Stops::getColumnName(size_t index) noexcept(false)
{
    assert(index < metaData_.size());
    return metaData_[index].colName_;
}
Stops::Stops(const Row &r, const ssize_t indexOffset) noexcept
{
    if(indexOffset < 0)
    {
        if(!r["id"].isNull())
        {
            id_=std::make_shared<std::string>(r["id"].as<std::string>());
        }
        if(!r["route_id"].isNull())
        {
            routeId_=std::make_shared<std::string>(r["route_id"].as<std::string>());
        }
        if(!r["name"].isNull())
        {
            name_=std::make_shared<std::string>(r["name"].as<std::string>());
        }
        if(!r["latitude"].isNull())
        {
            latitude_=std::make_shared<std::string>(r["latitude"].as<std::string>());
        }
        if(!r["longitude"].isNull())
        {
            longitude_=std::make_shared<std::string>(r["longitude"].as<std::string>());
        }
        if(!r["sequence_order"].isNull())
        {
            sequenceOrder_=std::make_shared<int32_t>(r["sequence_order"].as<int32_t>());
        }
    }
    else
    {
        size_t offset = (size_t)indexOffset;
        if(offset + 6 > r.size())
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
            routeId_=std::make_shared<std::string>(r[index].as<std::string>());
        }
        index = offset + 2;
        if(!r[index].isNull())
        {
            name_=std::make_shared<std::string>(r[index].as<std::string>());
        }
        index = offset + 3;
        if(!r[index].isNull())
        {
            latitude_=std::make_shared<std::string>(r[index].as<std::string>());
        }
        index = offset + 4;
        if(!r[index].isNull())
        {
            longitude_=std::make_shared<std::string>(r[index].as<std::string>());
        }
        index = offset + 5;
        if(!r[index].isNull())
        {
            sequenceOrder_=std::make_shared<int32_t>(r[index].as<int32_t>());
        }
    }

}

Stops::Stops(const Json::Value &pJson, const std::vector<std::string> &pMasqueradingVector) noexcept(false)
{
    if(pMasqueradingVector.size() != 6)
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
            routeId_=std::make_shared<std::string>(pJson[pMasqueradingVector[1]].asString());
        }
    }
    if(!pMasqueradingVector[2].empty() && pJson.isMember(pMasqueradingVector[2]))
    {
        dirtyFlag_[2] = true;
        if(!pJson[pMasqueradingVector[2]].isNull())
        {
            name_=std::make_shared<std::string>(pJson[pMasqueradingVector[2]].asString());
        }
    }
    if(!pMasqueradingVector[3].empty() && pJson.isMember(pMasqueradingVector[3]))
    {
        dirtyFlag_[3] = true;
        if(!pJson[pMasqueradingVector[3]].isNull())
        {
            latitude_=std::make_shared<std::string>(pJson[pMasqueradingVector[3]].asString());
        }
    }
    if(!pMasqueradingVector[4].empty() && pJson.isMember(pMasqueradingVector[4]))
    {
        dirtyFlag_[4] = true;
        if(!pJson[pMasqueradingVector[4]].isNull())
        {
            longitude_=std::make_shared<std::string>(pJson[pMasqueradingVector[4]].asString());
        }
    }
    if(!pMasqueradingVector[5].empty() && pJson.isMember(pMasqueradingVector[5]))
    {
        dirtyFlag_[5] = true;
        if(!pJson[pMasqueradingVector[5]].isNull())
        {
            sequenceOrder_=std::make_shared<int32_t>((int32_t)pJson[pMasqueradingVector[5]].asInt64());
        }
    }
}

Stops::Stops(const Json::Value &pJson) noexcept(false)
{
    if(pJson.isMember("id"))
    {
        dirtyFlag_[0]=true;
        if(!pJson["id"].isNull())
        {
            id_=std::make_shared<std::string>(pJson["id"].asString());
        }
    }
    if(pJson.isMember("route_id"))
    {
        dirtyFlag_[1]=true;
        if(!pJson["route_id"].isNull())
        {
            routeId_=std::make_shared<std::string>(pJson["route_id"].asString());
        }
    }
    if(pJson.isMember("name"))
    {
        dirtyFlag_[2]=true;
        if(!pJson["name"].isNull())
        {
            name_=std::make_shared<std::string>(pJson["name"].asString());
        }
    }
    if(pJson.isMember("latitude"))
    {
        dirtyFlag_[3]=true;
        if(!pJson["latitude"].isNull())
        {
            latitude_=std::make_shared<std::string>(pJson["latitude"].asString());
        }
    }
    if(pJson.isMember("longitude"))
    {
        dirtyFlag_[4]=true;
        if(!pJson["longitude"].isNull())
        {
            longitude_=std::make_shared<std::string>(pJson["longitude"].asString());
        }
    }
    if(pJson.isMember("sequence_order"))
    {
        dirtyFlag_[5]=true;
        if(!pJson["sequence_order"].isNull())
        {
            sequenceOrder_=std::make_shared<int32_t>((int32_t)pJson["sequence_order"].asInt64());
        }
    }
}

void Stops::updateByMasqueradedJson(const Json::Value &pJson,
                                            const std::vector<std::string> &pMasqueradingVector) noexcept(false)
{
    if(pMasqueradingVector.size() != 6)
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
            routeId_=std::make_shared<std::string>(pJson[pMasqueradingVector[1]].asString());
        }
    }
    if(!pMasqueradingVector[2].empty() && pJson.isMember(pMasqueradingVector[2]))
    {
        dirtyFlag_[2] = true;
        if(!pJson[pMasqueradingVector[2]].isNull())
        {
            name_=std::make_shared<std::string>(pJson[pMasqueradingVector[2]].asString());
        }
    }
    if(!pMasqueradingVector[3].empty() && pJson.isMember(pMasqueradingVector[3]))
    {
        dirtyFlag_[3] = true;
        if(!pJson[pMasqueradingVector[3]].isNull())
        {
            latitude_=std::make_shared<std::string>(pJson[pMasqueradingVector[3]].asString());
        }
    }
    if(!pMasqueradingVector[4].empty() && pJson.isMember(pMasqueradingVector[4]))
    {
        dirtyFlag_[4] = true;
        if(!pJson[pMasqueradingVector[4]].isNull())
        {
            longitude_=std::make_shared<std::string>(pJson[pMasqueradingVector[4]].asString());
        }
    }
    if(!pMasqueradingVector[5].empty() && pJson.isMember(pMasqueradingVector[5]))
    {
        dirtyFlag_[5] = true;
        if(!pJson[pMasqueradingVector[5]].isNull())
        {
            sequenceOrder_=std::make_shared<int32_t>((int32_t)pJson[pMasqueradingVector[5]].asInt64());
        }
    }
}

void Stops::updateByJson(const Json::Value &pJson) noexcept(false)
{
    if(pJson.isMember("id"))
    {
        if(!pJson["id"].isNull())
        {
            id_=std::make_shared<std::string>(pJson["id"].asString());
        }
    }
    if(pJson.isMember("route_id"))
    {
        dirtyFlag_[1] = true;
        if(!pJson["route_id"].isNull())
        {
            routeId_=std::make_shared<std::string>(pJson["route_id"].asString());
        }
    }
    if(pJson.isMember("name"))
    {
        dirtyFlag_[2] = true;
        if(!pJson["name"].isNull())
        {
            name_=std::make_shared<std::string>(pJson["name"].asString());
        }
    }
    if(pJson.isMember("latitude"))
    {
        dirtyFlag_[3] = true;
        if(!pJson["latitude"].isNull())
        {
            latitude_=std::make_shared<std::string>(pJson["latitude"].asString());
        }
    }
    if(pJson.isMember("longitude"))
    {
        dirtyFlag_[4] = true;
        if(!pJson["longitude"].isNull())
        {
            longitude_=std::make_shared<std::string>(pJson["longitude"].asString());
        }
    }
    if(pJson.isMember("sequence_order"))
    {
        dirtyFlag_[5] = true;
        if(!pJson["sequence_order"].isNull())
        {
            sequenceOrder_=std::make_shared<int32_t>((int32_t)pJson["sequence_order"].asInt64());
        }
    }
}

const std::string &Stops::getValueOfId() const noexcept
{
    static const std::string defaultValue = std::string();
    if(id_)
        return *id_;
    return defaultValue;
}
const std::shared_ptr<std::string> &Stops::getId() const noexcept
{
    return id_;
}
void Stops::setId(const std::string &pId) noexcept
{
    id_ = std::make_shared<std::string>(pId);
    dirtyFlag_[0] = true;
}
void Stops::setId(std::string &&pId) noexcept
{
    id_ = std::make_shared<std::string>(std::move(pId));
    dirtyFlag_[0] = true;
}
const typename Stops::PrimaryKeyType & Stops::getPrimaryKey() const
{
    assert(id_);
    return *id_;
}

const std::string &Stops::getValueOfRouteId() const noexcept
{
    static const std::string defaultValue = std::string();
    if(routeId_)
        return *routeId_;
    return defaultValue;
}
const std::shared_ptr<std::string> &Stops::getRouteId() const noexcept
{
    return routeId_;
}
void Stops::setRouteId(const std::string &pRouteId) noexcept
{
    routeId_ = std::make_shared<std::string>(pRouteId);
    dirtyFlag_[1] = true;
}
void Stops::setRouteId(std::string &&pRouteId) noexcept
{
    routeId_ = std::make_shared<std::string>(std::move(pRouteId));
    dirtyFlag_[1] = true;
}
void Stops::setRouteIdToNull() noexcept
{
    routeId_.reset();
    dirtyFlag_[1] = true;
}

const std::string &Stops::getValueOfName() const noexcept
{
    static const std::string defaultValue = std::string();
    if(name_)
        return *name_;
    return defaultValue;
}
const std::shared_ptr<std::string> &Stops::getName() const noexcept
{
    return name_;
}
void Stops::setName(const std::string &pName) noexcept
{
    name_ = std::make_shared<std::string>(pName);
    dirtyFlag_[2] = true;
}
void Stops::setName(std::string &&pName) noexcept
{
    name_ = std::make_shared<std::string>(std::move(pName));
    dirtyFlag_[2] = true;
}

const std::string &Stops::getValueOfLatitude() const noexcept
{
    static const std::string defaultValue = std::string();
    if(latitude_)
        return *latitude_;
    return defaultValue;
}
const std::shared_ptr<std::string> &Stops::getLatitude() const noexcept
{
    return latitude_;
}
void Stops::setLatitude(const std::string &pLatitude) noexcept
{
    latitude_ = std::make_shared<std::string>(pLatitude);
    dirtyFlag_[3] = true;
}
void Stops::setLatitude(std::string &&pLatitude) noexcept
{
    latitude_ = std::make_shared<std::string>(std::move(pLatitude));
    dirtyFlag_[3] = true;
}

const std::string &Stops::getValueOfLongitude() const noexcept
{
    static const std::string defaultValue = std::string();
    if(longitude_)
        return *longitude_;
    return defaultValue;
}
const std::shared_ptr<std::string> &Stops::getLongitude() const noexcept
{
    return longitude_;
}
void Stops::setLongitude(const std::string &pLongitude) noexcept
{
    longitude_ = std::make_shared<std::string>(pLongitude);
    dirtyFlag_[4] = true;
}
void Stops::setLongitude(std::string &&pLongitude) noexcept
{
    longitude_ = std::make_shared<std::string>(std::move(pLongitude));
    dirtyFlag_[4] = true;
}

const int32_t &Stops::getValueOfSequenceOrder() const noexcept
{
    static const int32_t defaultValue = int32_t();
    if(sequenceOrder_)
        return *sequenceOrder_;
    return defaultValue;
}
const std::shared_ptr<int32_t> &Stops::getSequenceOrder() const noexcept
{
    return sequenceOrder_;
}
void Stops::setSequenceOrder(const int32_t &pSequenceOrder) noexcept
{
    sequenceOrder_ = std::make_shared<int32_t>(pSequenceOrder);
    dirtyFlag_[5] = true;
}

void Stops::updateId(const uint64_t id)
{
}

const std::vector<std::string> &Stops::insertColumns() noexcept
{
    static const std::vector<std::string> inCols={
        "id",
        "route_id",
        "name",
        "latitude",
        "longitude",
        "sequence_order"
    };
    return inCols;
}

void Stops::outputArgs(drogon::orm::internal::SqlBinder &binder) const
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
        if(getRouteId())
        {
            binder << getValueOfRouteId();
        }
        else
        {
            binder << nullptr;
        }
    }
    if(dirtyFlag_[2])
    {
        if(getName())
        {
            binder << getValueOfName();
        }
        else
        {
            binder << nullptr;
        }
    }
    if(dirtyFlag_[3])
    {
        if(getLatitude())
        {
            binder << getValueOfLatitude();
        }
        else
        {
            binder << nullptr;
        }
    }
    if(dirtyFlag_[4])
    {
        if(getLongitude())
        {
            binder << getValueOfLongitude();
        }
        else
        {
            binder << nullptr;
        }
    }
    if(dirtyFlag_[5])
    {
        if(getSequenceOrder())
        {
            binder << getValueOfSequenceOrder();
        }
        else
        {
            binder << nullptr;
        }
    }
}

const std::vector<std::string> Stops::updateColumns() const
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
    if(dirtyFlag_[5])
    {
        ret.push_back(getColumnName(5));
    }
    return ret;
}

void Stops::updateArgs(drogon::orm::internal::SqlBinder &binder) const
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
        if(getRouteId())
        {
            binder << getValueOfRouteId();
        }
        else
        {
            binder << nullptr;
        }
    }
    if(dirtyFlag_[2])
    {
        if(getName())
        {
            binder << getValueOfName();
        }
        else
        {
            binder << nullptr;
        }
    }
    if(dirtyFlag_[3])
    {
        if(getLatitude())
        {
            binder << getValueOfLatitude();
        }
        else
        {
            binder << nullptr;
        }
    }
    if(dirtyFlag_[4])
    {
        if(getLongitude())
        {
            binder << getValueOfLongitude();
        }
        else
        {
            binder << nullptr;
        }
    }
    if(dirtyFlag_[5])
    {
        if(getSequenceOrder())
        {
            binder << getValueOfSequenceOrder();
        }
        else
        {
            binder << nullptr;
        }
    }
}
Json::Value Stops::toJson() const
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
    if(getRouteId())
    {
        ret["route_id"]=getValueOfRouteId();
    }
    else
    {
        ret["route_id"]=Json::Value();
    }
    if(getName())
    {
        ret["name"]=getValueOfName();
    }
    else
    {
        ret["name"]=Json::Value();
    }
    if(getLatitude())
    {
        ret["latitude"]=getValueOfLatitude();
    }
    else
    {
        ret["latitude"]=Json::Value();
    }
    if(getLongitude())
    {
        ret["longitude"]=getValueOfLongitude();
    }
    else
    {
        ret["longitude"]=Json::Value();
    }
    if(getSequenceOrder())
    {
        ret["sequence_order"]=getValueOfSequenceOrder();
    }
    else
    {
        ret["sequence_order"]=Json::Value();
    }
    return ret;
}

std::string Stops::toString() const
{
    return toJson().toStyledString();
}

Json::Value Stops::toMasqueradedJson(
    const std::vector<std::string> &pMasqueradingVector) const
{
    Json::Value ret;
    if(pMasqueradingVector.size() == 6)
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
            if(getRouteId())
            {
                ret[pMasqueradingVector[1]]=getValueOfRouteId();
            }
            else
            {
                ret[pMasqueradingVector[1]]=Json::Value();
            }
        }
        if(!pMasqueradingVector[2].empty())
        {
            if(getName())
            {
                ret[pMasqueradingVector[2]]=getValueOfName();
            }
            else
            {
                ret[pMasqueradingVector[2]]=Json::Value();
            }
        }
        if(!pMasqueradingVector[3].empty())
        {
            if(getLatitude())
            {
                ret[pMasqueradingVector[3]]=getValueOfLatitude();
            }
            else
            {
                ret[pMasqueradingVector[3]]=Json::Value();
            }
        }
        if(!pMasqueradingVector[4].empty())
        {
            if(getLongitude())
            {
                ret[pMasqueradingVector[4]]=getValueOfLongitude();
            }
            else
            {
                ret[pMasqueradingVector[4]]=Json::Value();
            }
        }
        if(!pMasqueradingVector[5].empty())
        {
            if(getSequenceOrder())
            {
                ret[pMasqueradingVector[5]]=getValueOfSequenceOrder();
            }
            else
            {
                ret[pMasqueradingVector[5]]=Json::Value();
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
    if(getRouteId())
    {
        ret["route_id"]=getValueOfRouteId();
    }
    else
    {
        ret["route_id"]=Json::Value();
    }
    if(getName())
    {
        ret["name"]=getValueOfName();
    }
    else
    {
        ret["name"]=Json::Value();
    }
    if(getLatitude())
    {
        ret["latitude"]=getValueOfLatitude();
    }
    else
    {
        ret["latitude"]=Json::Value();
    }
    if(getLongitude())
    {
        ret["longitude"]=getValueOfLongitude();
    }
    else
    {
        ret["longitude"]=Json::Value();
    }
    if(getSequenceOrder())
    {
        ret["sequence_order"]=getValueOfSequenceOrder();
    }
    else
    {
        ret["sequence_order"]=Json::Value();
    }
    return ret;
}

bool Stops::validateJsonForCreation(const Json::Value &pJson, std::string &err)
{
    if(pJson.isMember("id"))
    {
        if(!validJsonOfField(0, "id", pJson["id"], err, true))
            return false;
    }
    if(pJson.isMember("route_id"))
    {
        if(!validJsonOfField(1, "route_id", pJson["route_id"], err, true))
            return false;
    }
    if(pJson.isMember("name"))
    {
        if(!validJsonOfField(2, "name", pJson["name"], err, true))
            return false;
    }
    else
    {
        err="The name column cannot be null";
        return false;
    }
    if(pJson.isMember("latitude"))
    {
        if(!validJsonOfField(3, "latitude", pJson["latitude"], err, true))
            return false;
    }
    else
    {
        err="The latitude column cannot be null";
        return false;
    }
    if(pJson.isMember("longitude"))
    {
        if(!validJsonOfField(4, "longitude", pJson["longitude"], err, true))
            return false;
    }
    else
    {
        err="The longitude column cannot be null";
        return false;
    }
    if(pJson.isMember("sequence_order"))
    {
        if(!validJsonOfField(5, "sequence_order", pJson["sequence_order"], err, true))
            return false;
    }
    else
    {
        err="The sequence_order column cannot be null";
        return false;
    }
    return true;
}
bool Stops::validateMasqueradedJsonForCreation(const Json::Value &pJson,
                                               const std::vector<std::string> &pMasqueradingVector,
                                               std::string &err)
{
    if(pMasqueradingVector.size() != 6)
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
        else
        {
            err="The " + pMasqueradingVector[3] + " column cannot be null";
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
        else
        {
            err="The " + pMasqueradingVector[4] + " column cannot be null";
            return false;
        }
      }
      if(!pMasqueradingVector[5].empty())
      {
          if(pJson.isMember(pMasqueradingVector[5]))
          {
              if(!validJsonOfField(5, pMasqueradingVector[5], pJson[pMasqueradingVector[5]], err, true))
                  return false;
          }
        else
        {
            err="The " + pMasqueradingVector[5] + " column cannot be null";
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
bool Stops::validateJsonForUpdate(const Json::Value &pJson, std::string &err)
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
    if(pJson.isMember("route_id"))
    {
        if(!validJsonOfField(1, "route_id", pJson["route_id"], err, false))
            return false;
    }
    if(pJson.isMember("name"))
    {
        if(!validJsonOfField(2, "name", pJson["name"], err, false))
            return false;
    }
    if(pJson.isMember("latitude"))
    {
        if(!validJsonOfField(3, "latitude", pJson["latitude"], err, false))
            return false;
    }
    if(pJson.isMember("longitude"))
    {
        if(!validJsonOfField(4, "longitude", pJson["longitude"], err, false))
            return false;
    }
    if(pJson.isMember("sequence_order"))
    {
        if(!validJsonOfField(5, "sequence_order", pJson["sequence_order"], err, false))
            return false;
    }
    return true;
}
bool Stops::validateMasqueradedJsonForUpdate(const Json::Value &pJson,
                                             const std::vector<std::string> &pMasqueradingVector,
                                             std::string &err)
{
    if(pMasqueradingVector.size() != 6)
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
      if(!pMasqueradingVector[5].empty() && pJson.isMember(pMasqueradingVector[5]))
      {
          if(!validJsonOfField(5, pMasqueradingVector[5], pJson[pMasqueradingVector[5]], err, false))
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
bool Stops::validJsonOfField(size_t index,
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
                return true;
            }
            if(!pJson.isString())
            {
                err="Type error in the "+fieldName+" field";
                return false;
            }
            break;
        case 2:
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
                .from_bytes(pJson.asCString()).size() > 100)
            {
                err="String length exceeds limit for the " +
                    fieldName +
                    " field (the maximum value is 100)";
                return false;
            }
            break;
        case 3:
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
        case 4:
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
        case 5:
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
        default:
            err="Internal error in the server";
            return false;
    }
    return true;
}
