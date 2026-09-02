// Mirrors backend/src/utils/Roles.h and the seed data in
// backend/database/init.sql - keep these two in sync.
export const ROLES = {
  PASSENGER: 1,
  DRIVER: 2,
  CONDUCTOR: 3,
  ADMIN: 4,
}

export const ROLE_NAMES = {
  1: 'PASSENGER',
  2: 'DRIVER',
  3: 'CONDUCTOR',
  4: 'ADMIN',
}

export function roleName(roleId) {
  return ROLE_NAMES[roleId] || 'UNKNOWN'
}

export function roleHomePath(roleId) {
  switch (roleId) {
    case ROLES.ADMIN:
      return '/admin'
    case ROLES.DRIVER:
    case ROLES.CONDUCTOR:
      return '/driver'
    default:
      return '/'
  }
}
