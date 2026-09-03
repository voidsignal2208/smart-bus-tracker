

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

class Buses
{
  public:
    struct Cols
    {
        static const std::string _id;
        static const std::string _license_plate;
        static const std::string _capacity;
        static const std::string _status;
        static const std::string _created_at;
    };

    static const int primaryKeyNumber;
    static const std::string tableName;
    static const bool hasPrimaryKey;
    static const std::string primaryKeyName;
    using PrimaryKeyType = std::string;
    const PrimaryKeyType &getPrimaryKey() const;

    
    explicit Buses(const drogon::orm::Row &r, const ssize_t indexOffset = 0) noexcept;

    
    explicit Buses(const Json::Value &pJson) noexcept(false);

    
    Buses(const Json::Value &pJson, const std::vector<std::string> &pMasqueradingVector) noexcept(false);

    Buses() = default;

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

    
    
    const std::string &getValueOfLicensePlate() const noexcept;
    
    const std::shared_ptr<std::string> &getLicensePlate() const noexcept;
    
    void setLicensePlate(const std::string &pLicensePlate) noexcept;
    void setLicensePlate(std::string &&pLicensePlate) noexcept;

    
    
    const int32_t &getValueOfCapacity() const noexcept;
    
    const std::shared_ptr<int32_t> &getCapacity() const noexcept;
    
    void setCapacity(const int32_t &pCapacity) noexcept;

    
    
    const std::string &getValueOfStatus() const noexcept;
    
    const std::shared_ptr<std::string> &getStatus() const noexcept;
    
    void setStatus(const std::string &pStatus) noexcept;
    void setStatus(std::string &&pStatus) noexcept;
    void setStatusToNull() noexcept;

    
    
    const ::trantor::Date &getValueOfCreatedAt() const noexcept;
    
    const std::shared_ptr<::trantor::Date> &getCreatedAt() const noexcept;
    
    void setCreatedAt(const ::trantor::Date &pCreatedAt) noexcept;
    void setCreatedAtToNull() noexcept;


    static size_t getColumnNumber() noexcept {  return 5;  }
    static const std::string &getColumnName(size_t index) noexcept(false);

    Json::Value toJson() const;
    std::string toString() const;
    Json::Value toMasqueradedJson(const std::vector<std::string> &pMasqueradingVector) const;
    
  private:
    friend drogon::orm::Mapper<Buses>;
    friend drogon::orm::BaseBuilder<Buses, true, true>;
    friend drogon::orm::BaseBuilder<Buses, true, false>;
    friend drogon::orm::BaseBuilder<Buses, false, true>;
    friend drogon::orm::BaseBuilder<Buses, false, false>;
#ifdef __cpp_impl_coroutine
    friend drogon::orm::CoroMapper<Buses>;
#endif
    static const std::vector<std::string> &insertColumns() noexcept;
    void outputArgs(drogon::orm::internal::SqlBinder &binder) const;
    const std::vector<std::string> updateColumns() const;
    void updateArgs(drogon::orm::internal::SqlBinder &binder) const;
    
    void updateId(const uint64_t id);
    std::shared_ptr<std::string> id_;
    std::shared_ptr<std::string> licensePlate_;
    std::shared_ptr<int32_t> capacity_;
    std::shared_ptr<std::string> status_;
    std::shared_ptr<::trantor::Date> createdAt_;
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
    bool dirtyFlag_[5]={ false };
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
            sql += "license_plate,";
            ++parametersCount;
        }
        if(dirtyFlag_[2])
        {
            sql += "capacity,";
            ++parametersCount;
        }
        sql += "status,";
        ++parametersCount;
        if(!dirtyFlag_[3])
        {
            needSelection=true;
        }
        sql += "created_at,";
        ++parametersCount;
        if(!dirtyFlag_[4])
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
        else
        {
            sql +="default,";
        }
        if(dirtyFlag_[4])
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
