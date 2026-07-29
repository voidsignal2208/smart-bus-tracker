#pragma once

// Mirrors the seed data in database/init.sql:
// INSERT INTO roles (name) VALUES ('PASSENGER'), ('DRIVER'), ('CONDUCTOR'), ('ADMIN');
namespace Roles
{
    constexpr int PASSENGER = 1;
    constexpr int DRIVER = 2;
    constexpr int CONDUCTOR = 3;
    constexpr int ADMIN = 4;
}
