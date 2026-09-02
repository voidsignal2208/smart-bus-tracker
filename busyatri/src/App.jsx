import React from 'react'
import { Routes, Route } from 'react-router-dom'
import Home from './pages/Home'
import Admin from './pages/Admin'
import Passenger from './pages/Passenger'
import Driver from './pages/Driver'
import Login from './pages/Login'
import Track from './pages/Track'
import ProtectedRoute from './components/ProtectedRoute'
import { ROLES } from './lib/roles'

const App = () => {
  return (
    <Routes>
      <Route path='/' element={<Home />} />
      <Route path='/login' element={<Login />} />

      {/* Any logged-in user (any role) can search for and track buses. */}
      <Route
        path='/passenger'
        element={
          <ProtectedRoute>
            <Passenger />
          </ProtectedRoute>
        }
      />
      <Route
        path='/track'
        element={
          <ProtectedRoute>
            <Track />
          </ProtectedRoute>
        }
      />

      {/* Driver/Conductor console - matches StaffOnlyFilter on the backend. */}
      <Route
        path='/driver'
        element={
          <ProtectedRoute allowedRoles={[ROLES.DRIVER, ROLES.CONDUCTOR, ROLES.ADMIN]}>
            <Driver />
          </ProtectedRoute>
        }
      />

      {/* Admin dashboard - matches AdminOnlyFilter on the backend. */}
      <Route
        path='/admin'
        element={
          <ProtectedRoute allowedRoles={[ROLES.ADMIN]}>
            <Admin />
          </ProtectedRoute>
        }
      />
    </Routes>
  )
}
export default App
