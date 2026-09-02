import React from 'react'
import { Navigate } from 'react-router-dom'
import { useAuth } from '../context/AuthContext'

// Wrap a route's element with this to require login, and optionally a
// specific set of role ids (see lib/roles.js). Mirrors what the backend
// itself enforces (JwtAuthFilter / AdminOnlyFilter / StaffOnlyFilter) -
// this is purely a UI convenience so users see a redirect instead of a
// page full of failed requests. The backend is still the real gate.
const ProtectedRoute = ({ allowedRoles, children }) => {
  const { user } = useAuth()

  if (!user) {
    return <Navigate to='/login' replace />
  }

  if (allowedRoles && !allowedRoles.includes(user.roleId)) {
    return <Navigate to='/' replace />
  }

  return children
}

export default ProtectedRoute
