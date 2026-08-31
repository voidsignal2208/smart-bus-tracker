import React from 'react'
import { Routes, Route } from 'react-router-dom'
import Home from './pages/Home'
import Admin from './pages/Admin'
import Passenger from './pages/Passenger'
import Driver from './pages/Driver'
import Login from './pages/Login'
import Track from './pages/Track'

const App = () => {
  return (
    <Routes>
      <Route path='/' element={<Home />} />
      <Route path='/admin' element={<Admin />} />
      <Route path='/passenger' element={<Passenger />} />
      <Route path='/driver' element={<Driver />} />
      <Route path='/login' element={<Login/>}/>
      <Route path='/track' element={<Track/>}/>
    </Routes>
  )
}
export default App
