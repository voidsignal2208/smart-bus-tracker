import React, { useState } from 'react'
import { Link, useNavigate } from 'react-router-dom'
import { Menu, X, UserKey, BusFront, LogOut } from 'lucide-react'
import { useAuth } from '../context/AuthContext'
import { roleName } from '../lib/roles'

const Navbar = () => {
  const [isOpen, setIsOpen] = useState(false)
  const { user, logout } = useAuth()
  const navigate = useNavigate()

  const handleLogout = () => {
    logout()
    setIsOpen(false)
    navigate('/')
  }

  return (
    <div className='w-full fixed h-16 px-4 flex justify-between items-center bg-white z-50'>
      <Link to='/' className='text-black bg-white flex items-center text-xl'><BusFront size={30} />Bus <span className='text-lime-800'>Yatri</span></Link>

      <button className='md:hidden' onClick={() => setIsOpen(!isOpen)}>
        {isOpen ? <X size={22} /> : <Menu size={22} />}
      </button>

      <div className={`${isOpen ? 'flex' : 'hidden'} md:flex flex-col md:flex-row md:items-center gap-4 md:gap-8 font-bold absolute md:static top-16 left-0 w-full md:w-auto bg-white p-4 md:p-0`}>
        <Link to='/' className='hover:underline' onClick={() => setIsOpen(false)}>Home</Link>
        <span className='hover:underline'>Contact</span>

        {user ? (
          <>
            <span className='text-xs font-normal text-gray-500 md:border-l md:pl-4'>
              Signed in as {roleName(user.roleId)}
            </span>
            <button
              onClick={handleLogout}
              className='hover:underline flex items-center gap-2 border border-black rounded px-2 py-1 bg-stone-700 text-white'
            >
              <LogOut size={18} /> Logout
            </button>
          </>
        ) : (
          <Link
            to='/login'
            onClick={() => setIsOpen(false)}
            className='hover:underline flex items-center gap-2 border border-black rounded px-2 py-1 bg-lime-700 text-white w-fit'
          >
            <UserKey size={20} /> Login
          </Link>
        )}
      </div>
    </div>
  )
}

export default Navbar
