

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

class Users
{
  public:
    struct Cols
    {
        static const std::string _id;
        static const std::string _role_id;
        static const std::string _name;
        static const std::string _email;
        static const std::string _password_hash;
        static const std::string _phone;
        static const std::string _created_at;
        static const std::string _updated_at;
    };

    static const int primaryKeyNumber;
    static const std::string tableName;
    static const bool hasPrimaryKey;
    static const std::string primaryKeyName;
    using PrimaryKeyType = std::string;
    const PrimaryKeyType &getPrimaryKey() const;

    
    explicit Users(const drogon::orm::Row &r, const ssize_t indexOffset = 0) noexcept;

    
    explicit Users(const Json::Value &pJson) noexcept(false);

    
    Users(const Json::Value &pJson, const std::vector<std::string> &pMasqueradingVector) noexcept(false);

    Users() = default;

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

    
    
    const int32_t &getValueOfRoleId() const noexcept;
    
    const std::shared_ptr<int32_t> &getRoleId() const noexcept;
    
    void setRoleId(const int32_t &pRoleId) noexcept;
    void setRoleIdToNull() noexcept;

    
    
    const std::string &getValueOfName() const noexcept;
    
    const std::shared_ptr<std::string> &getName() const noexcept;
    
    void setName(const std::string &pName) noexcept;
    void setName(std::string &&pName) noexcept;

    
    
    const std::string &getValueOfEmail() const noexcept;
    
    const std::shared_ptr<std::string> &getEmail() const noexcept;
    
    void setEmail(const std::string &pEmail) noexcept;
    void setEmail(std::string &&pEmail) noexcept;

    
    
    const std::string &getValueOfPasswordHash() const noexcept;
    
    const std::shared_ptr<std::string> &getPasswordHash() const noexcept;
    
    void setPasswordHash(const std::string &pPasswordHash) noexcept;
    void setPasswordHash(std::string &&pPasswordHash) noexcept;

    
    
    const std::string &getValueOfPhone() const noexcept;
    
    const std::shared_ptr<std::string> &getPhone() const noexcept;
    
    void setPhone(const std::string &pPhone) noexcept;
    void setPhone(std::string &&pPhone) noexcept;
    void setPhoneToNull() noexcept;

    
    
    const ::trantor::Date &getValueOfCreatedAt() const noexcept;
    
    const std::shared_ptr<::trantor::Date> &getCreatedAt() const noexcept;
    
    void setCreatedAt(const ::trantor::Date &pCreatedAt) noexcept;
    void setCreatedAtToNull() noexcept;

    
    
    const ::trantor::Date &getValueOfUpdatedAt() const noexcept;
    
    const std::shared_ptr<::trantor::Date> &getUpdatedAt() const noexcept;
    
    void setUpdatedAt(const ::trantor::Date &pUpdatedAt) noexcept;
    void setUpdatedAtToNull() noexcept;


    static size_t getColumnNumber() noexcept {  return 8;  }
    static const std::string &getColumnName(size_t index) noexcept(false);

    Json::Value toJson() const;
    std::string toString() const;
    Json::Value toMasqueradedJson(const std::vector<std::string> &pMasqueradingVector) const;
    
  private:
    friend drogon::orm::Mapper<Users>;
    friend drogon::orm::BaseBuilder<Users, true, true>;
    friend drogon::orm::BaseBuilder<Users, true, false>;
    friend drogon::orm::BaseBuilder<Users, false, true>;
    friend drogon::orm::BaseBuilder<Users, false, false>;
#ifdef __cpp_impl_coroutine
    friend drogon::orm::CoroMapper<Users>;
#endif
    static const std::vector<std::string> &insertColumns() noexcept;
    void outputArgs(drogon::orm::internal::SqlBinder &binder) const;
    const std::vector<std::string> updateColumns() const;
    void updateArgs(drogon::orm::internal::SqlBinder &binder) const;
    
    void updateId(const uint64_t id);
    std::shared_ptr<std::string> id_;
    std::shared_ptr<int32_t> roleId_;
    std::shared_ptr<std::string> name_;
    std::shared_ptr<std::string> email_;
    std::shared_ptr<std::string> passwordHash_;
    std::shared_ptr<std::string> phone_;
    std::shared_ptr<::trantor::Date> createdAt_;
    std::shared_ptr<::trantor::Date> updatedAt_;
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
    bool dirtyFlag_[8]={ false };
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
            sql += "role_id,";
            ++parametersCount;
        }
        if(dirtyFlag_[2])
        {
            sql += "name,";
            ++parametersCount;
        }
        if(dirtyFlag_[3])
        {
            sql += "email,";
            ++parametersCount;
        }
        if(dirtyFlag_[4])
        {
            sql += "password_hash,";
            ++parametersCount;
        }
        if(dirtyFlag_[5])
        {
            sql += "phone,";
            ++parametersCount;
        }
        sql += "created_at,";
        ++parametersCount;
        if(!dirtyFlag_[6])
        {
            needSelection=true;
        }
        sql += "updated_at,";
        ++parametersCount;
        if(!dirtyFlag_[7])
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
        if(dirtyFlag_[6])
        {
            n = snprintf(placeholderStr,sizeof(placeholderStr),"$%d,",placeholder++);
            sql.append(placeholderStr, n);
        }
        else
        {
            sql +="default,";
        }
        if(dirtyFlag_[7])
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
