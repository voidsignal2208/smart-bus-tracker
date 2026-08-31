import React from 'react'
import { Star, MapPin } from 'lucide-react'

const BusRow = ({ index, busNo, arrivalTime, rating, onTrack }) => {
  return (
    <div className={`grid grid-cols-5 items-center px-6 py-4 ${index % 2 === 0 ? 'bg-amber-50' : 'bg-white'}`}>
      <div className='flex justify-start'>
        <span className='flex h-7 w-7 items-center justify-center rounded bg-stone-300 text-sm font-semibold text-gray-800'>
          {index}
        </span>
      </div>

      <div className='text-sm text-gray-800'>{busNo}</div>

      <div className='text-sm text-gray-800'>{arrivalTime}</div>

      <div className='flex gap-0.5'>
        {[1, 2, 3, 4, 5].map((n) => (
          <Star
            key={n}
            size={16}
            className={n <= rating ? 'fill-lime-800 text-lime-800' : 'text-gray-300'}
          />
        ))}
      </div>

      <div className='flex justify-start'>
        <button
          onClick={onTrack}
          className='flex items-center gap-1 rounded-full bg-lime-100 px-4 py-1.5 text-sm font-medium text-lime-900 hover:bg-lime-300'
        >
          <MapPin size={14} />
          Track
        </button>
      </div>
    </div>
  )
}

export default BusRow