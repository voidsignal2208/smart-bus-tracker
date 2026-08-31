import React from 'react'
import Navbar from '../components/Navbar'
import BusRow from '../components/Busrow'
import Footer from '../components/Footer'
import { MapPin, CalendarDays, BusFront } from 'lucide-react'
import { useSearchParams ,useNavigate } from 'react-router-dom'

const buses = [
  { busNo: '12345678', arrivalTime: '7:30 am', rating: 3 },
  { busNo: '38889348', arrivalTime: '9:00 am', rating: 2 },
  { busNo: '35548454', arrivalTime: '10:23 am', rating: 4 },
  { busNo: '04438218', arrivalTime: '11:11 am', rating: 5 },
  { busNo: '67676767', arrivalTime: '12:12 pm', rating: 3 },
  { busNo: '30112006', arrivalTime: '4:25 pm', rating: 5 },
  { busNo: '98765432', arrivalTime: '6:00 pm', rating: 1 },
]

const Passenger = () => {
  const navigate = useNavigate()
  const [searchParams] = useSearchParams()
  const from = searchParams.get('from') || 'Location 1'
  const to = searchParams.get('to') || 'Location 2'
  const rawDate = searchParams.get('date')
  const date = rawDate
    ? new Date(rawDate).toLocaleDateString('en-GB', { day: '2-digit', month: 'short', year: 'numeric' })
    : '21 Aug 2026'

  return (
    <div className='min-h-screen bg-amber-50 flex flex-col'>
      <Navbar />

      <div className='flex-1 pt-20 px-4 md:px-14 pb-10'>
        <div className='mx-auto max-w-5xl rounded-xl bg-white p-4 md:p-5 shadow-sm flex flex-col md:flex-row items-stretch md:items-center gap-3 divide-y md:divide-y-0 md:divide-x divide-gray-200'>
          <div className='flex-1 flex items-center gap-2 py-2 md:py-0 px-1 md:px-3'>
            <MapPin size={18} className='text-gray-500 shrink-0' />
            <div>
              <p className='text-[10px] font-semibold text-gray-500'>FROM</p>
              <p className='text-sm text-gray-800'>{from}</p>
            </div>
          </div>

          <div className='flex-1 flex items-center gap-2 py-2 md:py-0 px-1 md:px-3'>
            <MapPin size={18} className='text-gray-500 shrink-0' />
            <div>
              <p className='text-[10px] font-semibold text-gray-500'>TO</p>
              <p className='text-sm text-gray-800'>{to}</p>
            </div>
          </div>

          <div className='flex-1 flex items-center gap-2 py-2 md:py-0 px-1 md:px-3'>
            <CalendarDays size={18} className='text-gray-500 shrink-0' />
            <div>
              <p className='text-[10px] font-semibold text-gray-500'>DATE</p>
              <p className='text-sm text-gray-800'>{date}</p>
            </div>
          </div>
        </div>

        <div className='mx-auto mt-6 max-w-5xl rounded-xl border border-gray-200 bg-white overflow-hidden'>
          <div className='flex flex-col gap-1 px-6 py-4 md:flex-row md:items-baseline md:justify-between'>
            <h2 className='text-sm font-semibold text-gray-500'>LIST OF BUSES</h2>
            <div className='text-sm text-gray-800'>
              <span className='font-medium'>{from} ⇄ {to}</span>
              <span className='block text-xs text-gray-400 md:ml-3 md:inline'>{date}</span>
            </div>
          </div>

          <div className='overflow-x-auto'>
            <div className='min-w-160'>
              <div className='grid grid-cols-5 bg-stone-500 px-6 py-3 text-sm font-semibold text-white'>
                <div>S No.</div>
                <div>Bus No.</div>
                <div>Arrival Time</div>
                <div>Rating</div>
                <div>Route</div>
              </div>

              {buses.map((bus, i) => (
                <BusRow
                  key={bus.busNo}
                  index={i + 1}
                  busNo={bus.busNo}
                  arrivalTime={bus.arrivalTime}
                  rating={bus.rating}
                  onTrack={() => navigate(`/track?busNo=${bus.busNo}&from=${from}&to=${to}`)}
                />
              ))}
            </div>
          </div>
        </div>
      </div>
      <Footer/>
    </div>
  )
}

export default Passenger