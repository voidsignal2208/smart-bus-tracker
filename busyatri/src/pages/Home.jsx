import React, { useState } from 'react'
import Navbar from '../components/Navbar'
import { BusFront, MapPin, CalendarDays, Search, Route } from 'lucide-react'
import { useNavigate } from 'react-router-dom'
import Footer from '../components/Footer'

const Home = () => {
  const navigate = useNavigate()
  const [from, setFrom] = useState('')
  const [to, setTo] = useState('')
  const [date, setDate] = useState('')

  const SubmitHandler = (e) => {
    e.preventDefault()
    const params = new URLSearchParams({ from, to, date }).toString()
    navigate(`/passenger?${params}`)
  }

  return (
    <div className='min-h-screen bg-amber-50'>
      <Navbar />
      <div className='pt-15'>
        <div className='relative bg-amber-100 px-5 py-12 md:px-14 md:py-20'>
        <div className='absolute right-0 top-0 hidden h-full w-1/2 md:flex items-center justify-center overflow-hidden opacity-50'>
          <img
            src='https://images.unsplash.com/photo-1736117705678-4d7d49850205?q=80&w=2072&auto=format&fit=crop&ixlib=rb-4.1.0&ixid=M3wxMjA3fDB8MHxwaG90by1wYWdlfHx8fGVufDB8fHx8fA%3D%3D'
            className='h-full w-full object-cover'
           />
        </div>

          <div className='relative z-10 max-w-xl'>
            <p className='mb-5 text-xs text-gray-600'>
              ● LIVE NETWORK <span className='mx-3'>|</span> active buses across cities
            </p>
            <h1 className='text-4xl font-bold leading-tight text-gray-900 md:text-6xl'>
              Know Where <br />
              Your Bus Is <br />
              <span className='text-lime-800'>Before You Wait</span>
            </h1>
            <p className='mt-5 max-w-md text-gray-700'>
              Real-time bus tracking, accurate arrival time and alerts — all in one place.
            </p>
            <div className='mt-5 flex gap-5 text-sm font-medium text-gray-800'>
              <span className='flex items-center gap-2'><BusFront size={18} /> Track your bus</span>
              <span className='flex items-center gap-2'><Route size={18} /> Explore routes</span>
            </div>
          </div>

          <form className='relative z-20 mx-auto mt-10 w-full max-w-sm rounded-xl bg-white p-5 md:absolute md:-bottom-17.5 md:left-1/2 md:mt-0 md:-translate-x-1/2 shadow-lg' onSubmit={SubmitHandler}>
            <p className='text-xs text-gray-500'>PLAN YOUR JOURNEY</p>
            <h2 className='mb-4 text-xl font-bold'>Find your bus</h2>

            <label className='mb-1 block text-xs'>FROM</label>
            <div className='mb-3 flex items-center gap-2 rounded border border-gray-300 px-3 py-3'>
              <MapPin size={20} />
              <input type='text' placeholder='FROM' value={from} onChange={(e) => setFrom(e.target.value)} className='w-full outline-none' />
            </div>

            <label className='mb-1 block text-xs'>TO</label>
            <div className='mb-3 flex items-center gap-2 rounded border border-gray-300 px-3 py-3'>
              <MapPin size={20} />
              <input type='text' placeholder='TO' value={to} onChange={(e) => setTo(e.target.value)} className='w-full outline-none' />
            </div>

            <label className='mb-1 block text-xs'>DATE</label>
            <div className='flex items-center gap-2 rounded border border-gray-300 px-3 py-3'>
              <CalendarDays size={20} />
              <input type='date' value={date} onChange={(e) => setDate(e.target.value)} className='w-full outline-none' />
            </div>

            <button className='mt-4 flex w-full items-center justify-center gap-2 rounded bg-lime-800 py-3 font-bold text-white'>
              <Search size={20} />
              Search
            </button>
          </form>
        </div>

        <div className='px-5 pb-10 pt-32 md:px-14'>
          <h2 className='text-2xl font-bold'>How BusYatri works</h2>
          <div className='mt-6 grid gap-6 md:grid-cols-3'>
            <div>
              <Route className='mb-2 text-lime-800' size={30} />
              <h3 className='font-bold'>Choose your route</h3>
              <p className='text-sm text-gray-600'>Select your starting point and destination.</p>
            </div>
            <div>
              <BusFront className='mb-2 text-lime-800' size={30} />
              <h3 className='font-bold'>Track your bus</h3>
              <p className='text-sm text-gray-600'>Follow your bus in real time.</p>
            </div>
            <div>
              <CalendarDays className='mb-2 text-lime-800' size={30} />
              <h3 className='font-bold'>Get updates</h3>
              <p className='text-sm text-gray-600'>Receive arrival and service alerts.</p>
            </div>
          </div>
        </div>
      </div>
      <Footer/>
    </div>
  )
}

export default Home