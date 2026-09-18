import React, { createContext, useContext, useState, useEffect, useCallback } from 'react'
import { getToken, setToken as saveToken, clearToken, getUserFromToken } from '../services/api'
import { authService } from '../services/authService'

const AuthContext = createContext(null)

export const ROLES = {
  PASSENGER: 1,
  DRIVER: 2,
  CONDUCTOR: 3,
  ADMIN: 4,
}

export function AuthProvider({ children }) {
  const [user, setUser] = useState(null)
  const [loading, setLoading] = useState(true)

  // Restore session from localStorage on mount
  useEffect(() => {
    const token = getToken()
    if (token) {
      const decoded = getUserFromToken()
      if (decoded) {
        setUser({ ...decoded, token })
      } else {
        clearToken()
      }
    }
    setLoading(false)
  }, [])

  const login = useCallback(async (email, password) => {
    const data = await authService.login(email, password)
    if (!data?.token) throw new Error('The server did not return a login token')
    saveToken(data.token)
    const decoded = getUserFromToken()
    if (!decoded) {
      clearToken()
      throw new Error('The server returned an invalid login token')
    }
    // The signed token is the source of truth for access control. Do not
    // trust a separately returned role value when deciding where to route.
    const userData = { ...decoded, token: data.token }
    setUser(userData)
    return userData
  }, [])

  const logout = useCallback(() => {
    clearToken()
    setUser(null)
  }, [])

  const value = {
    user,
    loading,
    isAuthenticated: !!user,
    login,
    logout,
  }

  return <AuthContext.Provider value={value}>{children}</AuthContext.Provider>
}

export function useAuth() {
  const ctx = useContext(AuthContext)
  if (!ctx) throw new Error('useAuth must be used within AuthProvider')
  return ctx
}
