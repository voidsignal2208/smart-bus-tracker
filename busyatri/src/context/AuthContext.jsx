import React, { createContext, useContext, useState, useCallback } from 'react'
import { loginUser, registerUser, getToken, setToken as persistToken, ApiError } from '../lib/api'
import { decodeJwt, isTokenExpired } from '../lib/jwt'

const AuthContext = createContext(null)

function userFromToken(token) {
  if (!token || isTokenExpired(token)) return null
  const claims = decodeJwt(token)
  if (!claims) return null
  return {
    id: claims.user_id,
    roleId: Number(claims.role_id),
    token,
  }
}

export function AuthProvider({ children }) {
  const [user, setUser] = useState(() => userFromToken(getToken()))

  const login = useCallback(async (email, password) => {
    const data = await loginUser({ email, password })
    persistToken(data.token)
    const nextUser = userFromToken(data.token)
    setUser(nextUser)
    return nextUser
  }, [])

  const register = useCallback(async (name, email, password) => {
    // Registration never returns a token (self-registration is always
    // PASSENGER, per AuthService.cc) - log in right after so the caller
    // ends up authenticated in one step.
    await registerUser({ name, email, password })
    return login(email, password)
  }, [login])

  const logout = useCallback(() => {
    persistToken(null)
    setUser(null)
  }, [])

  return (
    <AuthContext.Provider value={{ user, login, register, logout }}>
      {children}
    </AuthContext.Provider>
  )
}

export function useAuth() {
  const ctx = useContext(AuthContext)
  if (!ctx) throw new Error('useAuth must be used within an AuthProvider')
  return ctx
}

export { ApiError }
