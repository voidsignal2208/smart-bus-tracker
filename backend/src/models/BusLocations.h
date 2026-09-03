

#pragma once
#include <drogon/orm/Result.h>
#include <drogon/orm/Row.h>
#include <drogon/orm/Field.h>
#include <drogon/orm/SqlBinder.h>
#include <drogon/orm/Mapper.h>
#include <drogon/orm/BaseBuilder.h>
#ifdef __cpp_impl_coroutine
#include <drogon/orm/CoroMapper.h>
#endif
#include <trantor/utils/Date.h>
#include <trantor/utils/Logger.h>
#include <json/json.h>
#include <string>
#include <string_view>
#include <memory>
#include <vector>
#include <tuple>
#include <stdint.h>
#include <iostream>

namespace drogon
{
namespace orm
{
class DbClient;
using DbClientPtr = std::shared_ptr<DbClient>;
}
}
namespace drogon_model
{
namespace postgres
{

class BusLocations
{
  public:
    struct Cols
    {
        static const std::string _id;
        static const std::string _bus_id;
        static const std::string _latitude;
        static const std::string _longitude;
        static const std::string _speed;
        static const std::string _timestamp;
    };

    static const int primaryKeyNumber;
    static const std::string tableName;
    static const bool hasPrimaryKey;
    static const std::string primaryKeyName;
    using PrimaryKeyType = std::string;
    const PrimaryKeyType &getPrimaryKey() const;

    
    explicit BusLocations(const drogon::orm::Row &r, const ssize_t indexOffset = 0) noexcept;

    
    explicit BusLocations(const Json::Value &pJson) noexcept(false);

    
    BusLocations(const Json::Value &pJson, const std::vector<std::string> &pMasqueradingVector) noexcept(false);

    BusLocations() = default;

    void updateByJson(const Json::Value &pJson) noexcept(false);
    void updateByMasqueradedJson(const Json::Value &pJson,
                                 const std::vector<std::string> &pMasqueradingVector) noexcept(false);
    static bool validateJsonForCreation(const Json::Value &pJson, std::string &err);
    static bool validateMasqueradedJsonForCreation(const Json::Value &,
                                                const std::vector<std::string> &pMasqueradingVector,
                                                    std::string &err);
    static bool validateJsonForUpdate(const Json::Value &pJson, std::string &err);
    static bool validateMasqueradedJsonForUpdate(const Json::Value &,
                                          const std::vector<std::string> &pMasqueradingVector,
                                          std::string &err);
    static bool validJsonOfField(size_t index,
                          const std::string &fieldName,
                          const Json::Value &pJson,
                          std::string &err,
                          bool isForCreation);

    
    
    const std::string &getValueOfId() const noexcept;
    
    const std::shared_ptr<std::string> &getId() const noexcept;
    
    void setId(const std::string &pId) noexcept;
    void setId(std::string &&pId) noexcept;

    
    
    const std::string &getValueOfBusId() const noexcept;
    
    const std::shared_ptr<std::string> &getBusId() const noexcept;
    
    void setBusId(const std::string &pBusId) noexcept;
    void setBusId(std::string &&pBusId) noexcept;
    void setBusIdToNull() noexcept;

    
    
    const std::string &getValueOfLatitude() const noexcept;
    
    const std::shared_ptr<std::string> &getLatitude() const noexcept;
    
    void setLatitude(const std::string &pLatitude) noexcept;
    void setLatitude(std::string &&pLatitude) noexcept;

    
    
    const std::string &getValueOfLongitude() const noexcept;
    
    const std::shared_ptr<std::string> &getLongitude() const noexcept;
    
    void setLongitude(const std::string &pLongitude) noexcept;
    void setLongitude(std::string &&pLongitude) noexcept;

    
    
    const std::string &getValueOfSpeed() const noexcept;
    
    const std::shared_ptr<std::string> &getSpeed() const noexcept;
    
    void setSpeed(const std::string &pSpeed) noexcept;
    void setSpeed(std::string &&pSpeed) noexcept;
    void setSpeedToNull() noexcept;

    
    
    const ::trantor::Date &getValueOfTimestamp() const noexcept;
    
    const std::shared_ptr<::trantor::Date> &getTimestamp() const noexcept;
    
    void setTimestamp(const ::trantor::Date &pTimestamp) noexcept;
    void setTimestampToNull() noexcept;


    static size_t getColumnNumber() noexcept {  return 6;  }
    static const std::string &getColumnName(size_t index) noexcept(false);

    Json::Value toJson() const;
    std::string toString() const;
    Json::Value toMasqueradedJson(const std::vector<std::string> &pMasqueradingVector) const;
    
  private:
    friend drogon::orm::Mapper<BusLocations>;
    friend drogon::orm::BaseBuilder<BusLocations, true, true>;
    friend drogon::orm::BaseBuilder<BusLocations, true, false>;
    friend drogon::orm::BaseBuilder<BusLocations, false, true>;
    friend drogon::orm::BaseBuilder<BusLocations, false, false>;
#ifdef __cpp_impl_coroutine
    friend drogon::orm::CoroMapper<BusLocations>;
#endif
    static const std::vector<std::string> &insertColumns() noexcept;
    void outputArgs(drogon::orm::internal::SqlBinder &binder) const;
    const std::vector<std::string> updateColumns() const;
    void updateArgs(drogon::orm::internal::SqlBinder &binder) const;
    
    void updateId(const uint64_t id);
    std::shared_ptr<std::string> id_;
    std::shared_ptr<std::string> busId_;
    std::shared_ptr<std::string> latitude_;
    std::shared_ptr<std::string> longitude_;
    std::shared_ptr<std::string> speed_;
    std::shared_ptr<::trantor::Date> timestamp_;
    struct MetaData
    {
        const std::string colName_;
        const std::string colType_;
        const std::string colDatabaseType_;
        const ssize_t colLength_;
        const bool isAutoVal_;
        const bool isPrimaryKey_;
        const bool notNull_;
    };
    static const std::vector<MetaData> metaData_;
    bool dirtyFlag_[6]={ false };
  public:
    static const std::string &sqlForFindingByPrimaryKey()
    {
        static const std::string sql="select * from " + tableName + " where id = $1";
        return sql;
    }

    static const std::string &sqlForDeletingByPrimaryKey()
    {
        static const std::string sql="delete from " + tableName + " where id = $1";
        return sql;
    }
    std::string sqlForInserting(bool &needSelection) const
    {
        std::string sql="insert into " + tableName + " (";
        size_t parametersCount = 0;
        needSelection = false;
        sql += "id,";
        ++parametersCount;
        if(!dirtyFlag_[0])
        {
            needSelection=true;
        }
        if(dirtyFlag_[1])
        {
            sql += "bus_id,";
            ++parametersCount;
        }
        if(dirtyFlag_[2])
        {
            sql += "latitude,";
            ++parametersCount;
        }
        if(dirtyFlag_[3])
        {
            sql += "longitude,";
            ++parametersCount;
        }
        if(dirtyFlag_[4])
        {
            sql += "speed,";
            ++parametersCount;
        }
        sql += "timestamp,";
        ++parametersCount;
        if(!dirtyFlag_[5])
        {
            needSelection=true;
        }
        if(parametersCount > 0)
        {
            sql[sql.length()-1]=')';
            sql += " values (";
        }
        else
            sql += ") values (";

        int placeholder=1;
        char placeholderStr[64];
        size_t n=0;
        if(dirtyFlag_[0])
        {
            n = snprintf(placeholderStr,sizeof(placeholderStr),"$%d,",placeholder++);
            sql.append(placeholderStr, n);
        }
        else
        {
            sql +="default,";
        }
        if(dirtyFlag_[1])
        {
            n = snprintf(placeholderStr,sizeof(placeholderStr),"$%d,",placeholder++);
            sql.append(placeholderStr, n);
        }
        if(dirtyFlag_[2])
        {
            n = snprintf(placeholderStr,sizeof(placeholderStr),"$%d,",placeholder++);
            sql.append(placeholderStr, n);
        }
        if(dirtyFlag_[3])
        {
            n = snprintf(placeholderStr,sizeof(placeholderStr),"$%d,",placeholder++);
            sql.append(placeholderStr, n);
        }
        if(dirtyFlag_[4])
        {
            n = snprintf(placeholderStr,sizeof(placeholderStr),"$%d,",placeholder++);
            sql.append(placeholderStr, n);
        }
        if(dirtyFlag_[5])
        {
            n = snprintf(placeholderStr,sizeof(placeholderStr),"$%d,",placeholder++);
            sql.append(placeholderStr, n);
        }
        else
        {
            sql +="default,";
        }
        if(parametersCount > 0)
        {
            sql.resize(sql.length() - 1);
        }
        if(needSelection)
        {
            sql.append(") returning *");
        }
        else
        {
            sql.append(1, ')');
        }
        LOG_TRACE << sql;
        return sql;
    }
};
} 
} 
