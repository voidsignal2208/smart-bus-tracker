import React from 'react'
import { Routes, Route, Navigate } from 'react-router-dom'
import { AuthProvider, ROLES, useAuth } from './context/AuthContext'
import Home from './pages/Home'
import Admin from './pages/Admin'
import Passenger from './pages/Passenger'
import Driver from './pages/Driver'
import Login from './pages/Login'
import Register from './pages/Register'
import Track from './pages/Track'

const App = () => {
  return (
    <AuthProvider>
      <Routes>
        <Route path='/' element={<Home />} />
        <Route path='/admin' element={<RequireRole role={ROLES.ADMIN}><Admin /></RequireRole>} />
        <Route path='/passenger' element={<Passenger />} />
        <Route path='/driver' element={<RequireRole role={ROLES.DRIVER}><Driver /></RequireRole>} />
        <Route path='/login' element={<Login/>}/>
        <Route path='/register' element={<Register/>}/>
        <Route path='/track' element={<Track/>}/>
      </Routes>
    </AuthProvider>
  )
}

const RequireRole = ({ role, children }) => {
  const { user, loading } = useAuth()
  if (loading) return <div className="min-h-screen grid place-items-center text-sm text-gray-500">Checking session…</div>
  if (!user) return <Navigate to="/login" replace />
  if (Number(user.roleId) !== role) return <Navigate to="/" replace />
  return children
}
export default App
