import React, { useState } from 'react'
import { Link } from 'react-router-dom'
import { Menu, X, UserKey, BusFront } from 'lucide-react'

const Navbar = () => {
  const [isOpen, setIsOpen] = useState(false)

  return (
    <div className='w-full fixed h-16 px-4 flex justify-between items-center bg-white z-50'>
      <h1 className='text-black bg-white flex items-center text-xl'><BusFront size={30} />Bus <span className='text-lime-800'>Yatri</span></h1>

      <button className='md:hidden' onClick={() => setIsOpen(!isOpen)}>
        {isOpen ? <X size={22} /> : <Menu size={22} />}
      </button>

      <div className={`${isOpen ? 'flex' : 'hidden'} md:flex flex-col md:flex-row gap-4 md:gap-8 font-bold absolute md:static top-16 left-0 w-full md:w-auto bg-white p-4 md:p-0`}>
        <Link to='/' className='hover:underline'>Home</Link>
        <span className='hover:underline'>Contact</span>
        <span className='hover:underline flex items-center gap-2 border border-black rounded px-2 py-1 bg-lime-700 text-white'><UserKey size={20} /><Link to={'/login'}>Login</Link></span>
      </div>
    </div>
  )
}

export default Navbar