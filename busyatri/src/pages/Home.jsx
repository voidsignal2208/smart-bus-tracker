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

  const [errors, setErrors] = useState({})

  const SubmitHandler = (e) => {
    e.preventDefault()
    const newErrors = {}
    if (!from.trim()) newErrors.from = 'Please enter a starting location.'
    if (!to.trim()) newErrors.to = 'Please enter a destination.'
    if (!date) newErrors.date = 'Please select a date.'
    setErrors(newErrors)
    if (Object.keys(newErrors).length > 0) return
    const params = new URLSearchParams({ from: from.trim(), to: to.trim(), date }).toString()
    navigate(`/passenger?${params}`)
  }

  return (
    <div className='min-h-screen bg-amber-50'>
      <Navbar />
      <div className='pt-15'>

        {/* Hero + Search Form */}
        <div className='relative bg-amber-100 px-5 py-12 md:px-14 md:py-20'>

          {/* Background image — right half, desktop only */}
          <div className='absolute right-0 top-0 hidden h-full w-1/2 md:flex items-center justify-center overflow-hidden opacity-40'>
            <img
              src='https://images.unsplash.com/photo-1736117705678-4d7d49850205?q=80&w=2072&auto=format&fit=crop&ixlib=rb-4.1.0&ixid=M3wxMjA3fDB8MHxwaG90by1wYWdlfHx8fGVufDB8fHx8fA%3D%3D'
              className='h-full w-full object-cover'
            />
          </div>

          <div className='relative z-10 flex flex-col md:flex-row md:items-start md:gap-12'>
            {/* Left — Hero text */}
            <div className='flex-1 max-w-xl'>
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

            {/* Right — Search Form (inline, no absolute positioning) */}
            <form
              className='relative z-20 mt-10 md:mt-0 w-full max-w-sm rounded-xl bg-white p-5 shadow-lg shrink-0'
              onSubmit={SubmitHandler}
            >
              <p className='text-xs text-gray-500'>PLAN YOUR JOURNEY</p>
              <h2 className='mb-4 text-xl font-bold'>Find your bus</h2>

              <label className='mb-1 block text-xs'>FROM <span className='text-red-500'>*</span></label>
              <div className={`mb-1 flex items-center gap-2 rounded border px-3 py-3 ${errors.from ? 'border-red-400 bg-red-50' : 'border-gray-300'}`}>
                <MapPin size={20} />
                <input
                  type='text'
                  placeholder='e.g. MNNIT Allahabad'
                  value={from}
                  onChange={(e) => { setFrom(e.target.value); setErrors((p) => ({ ...p, from: '' })) }}
                  className='w-full outline-none bg-transparent'
                />
              </div>
              {errors.from && <p className='text-red-500 text-[11px] mb-2'>{errors.from}</p>}

              <label className='mb-1 block text-xs'>TO <span className='text-red-500'>*</span></label>
              <div className={`mb-1 flex items-center gap-2 rounded border px-3 py-3 ${errors.to ? 'border-red-400 bg-red-50' : 'border-gray-300'}`}>
                <MapPin size={20} />
                <input
                  type='text'
                  placeholder='e.g. Prayagraj Junction'
                  value={to}
                  onChange={(e) => { setTo(e.target.value); setErrors((p) => ({ ...p, to: '' })) }}
                  className='w-full outline-none bg-transparent'
                />
              </div>
              {errors.to && <p className='text-red-500 text-[11px] mb-2'>{errors.to}</p>}

              <label className='mb-1 block text-xs'>DATE <span className='text-red-500'>*</span></label>
              <div className={`mb-1 flex items-center gap-2 rounded border px-3 py-3 ${errors.date ? 'border-red-400 bg-red-50' : 'border-gray-300'}`}>
                <CalendarDays size={20} />
                <input
                  type='date'
                  value={date}
                  onChange={(e) => { setDate(e.target.value); setErrors((p) => ({ ...p, date: '' })) }}
                  className='w-full outline-none bg-transparent text-gray-700'
                />
              </div>
              {errors.date && <p className='text-red-500 text-[11px] mb-2'>{errors.date}</p>}

              <button className='mt-4 flex w-full items-center justify-center gap-2 rounded bg-lime-800 py-3 font-bold text-white hover:bg-lime-900 transition'>
                <Search size={20} />
                Search Buses
              </button>
            </form>
          </div>
        </div>

        {/* How it works section — no overlap issue now */}
        <div className='px-5 pb-10 pt-12 md:px-14'>
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
