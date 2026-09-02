import React from 'react'
import { MapPin } from 'lucide-react'

const STATUS_STYLES = {
  ACTIVE: 'bg-lime-100 text-lime-900',
  INACTIVE: 'bg-gray-200 text-gray-700',
  MAINTENANCE: 'bg-amber-100 text-amber-900',
}

const BusRow = ({ index, licensePlate, routeName, capacity, status, onTrack }) => {
  const badgeClass = STATUS_STYLES[status] || 'bg-gray-200 text-gray-700'

  return (
    <div className={`grid grid-cols-5 items-center px-6 py-4 ${index % 2 === 0 ? 'bg-amber-50' : 'bg-white'}`}>
      <div className='flex justify-start'>
        <span className='flex h-7 w-7 items-center justify-center rounded bg-stone-300 text-sm font-semibold text-gray-800'>
          {index}
        </span>
      </div>

      <div>
        <div className='text-sm text-gray-800 font-medium'>{licensePlate}</div>
        {routeName && <div className='text-xs text-gray-500'>{routeName}</div>}
      </div>

      <div className='text-sm text-gray-800'>{capacity} seats</div>

      <div>
        <span className={`inline-block rounded-full px-3 py-1 text-xs font-semibold ${badgeClass}`}>
          {status}
        </span>
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
