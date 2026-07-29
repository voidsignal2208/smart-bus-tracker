# 🚌 Real-Time Smart Bus Tracking Platform

An enterprise-grade, ultra-low-latency real-time bus tracking system. Built with a **C++ backend** to handle high-throughput GPS ingestion and WebSocket broadcasting, paired with a modern **React/TypeScript frontend**.

## 📖 Project Overview

This platform provides real-time location tracking of city buses. It is designed to handle thousands of concurrent 5-second GPS pings from bus drivers while simultaneously broadcasting those updates via WebSockets to thousands of tracking passengers with millisecond latency.

To achieve maximum performance and minimal hardware footprint, the backend is powered by **Drogon** (a high-performance C++14/17/20 asynchronous web framework), utilizing Redis for fast in-memory location caching and asynchronous background batching for PostgreSQL persistence.

## ✨ Key Features

- **Live Tracking Engine**: Sub-second latency WebSocket broadcasts of bus movements on a live map.
- **Smart Roles**: Distinct interfaces for Passengers, Drivers, Conductors, and System Administrators.
- **Fleet & Route Management**: Admin dashboard to manage buses, define routes, plot stops, and assign staff shifts.
- **Passenger Portal**: Search for routes, view live bus locations, and see Estimated Time of Arrival (ETA) for upcoming stops.
- **Optimized Persistence**: In-memory C++ queuing system that batches live location data into PostgreSQL to prevent database thrashing.

## 🛠️ Technology Stack

### Backend (Ultra-High Performance)
- **Language**: C++17
- **Framework**: Drogon (event-driven, asynchronous web framework)
- **Authentication**: JWT (`jwt-cpp`)
- **Real-Time**: WebSockets
- **Build System**: CMake

### Frontend (Modern Web)
- **Library**: React 18
- **Language**: TypeScript
- **Styling**: Tailwind CSS
- **State Management**: React Query / Zustand
- **Maps**: Google Maps API

### Infrastructure & Data
- **Primary Database**: PostgreSQL (Drogon Async ORM)
- **In-Memory Cache**: Redis (`hiredis`)
- **Deployment**: Docker, Docker Compose, Nginx

## 📐 Architecture: Real-Time Location Flow

```text
[Driver Device]
      │ (POST /location every 5s)
      ▼
[C++ Drogon API] ──(Async Write)──> [Redis Cache]
      │                                 │
   (Pushes to Thread-Safe Queue)        │ (State source of truth)
      │                                 │
      ▼                                 ▼
[C++ Background Worker]         [C++ WebSocket Server]
      │                                 │
   (60s Batch Insert)                   │ (Broadcasts 5s updates)
      ▼                                 ▼
[PostgreSQL DB]                   [Passenger Browser]
```

## 📂 Project Structure

```text
smart-bus-tracker/
├── backend/            # C++ backend source code (Drogon)
│   ├── CMakeLists.txt
│   ├── src/            # Controllers, Services, Models, Repositories
│   └── config.json     # DB/Redis configs
├── frontend/           # React frontend source code
│   ├── src/            # Components, Hooks, Features, Types
│   └── package.json
└── infra/              # Docker Compose and Nginx configurations
```

## 🚀 Getting Started

### Prerequisites
- **C++ Environment**: C++17 compiler (GCC/Clang), CMake (3.15+)
- **Libraries**: `jsoncpp`, `uuid`, `zlib`, `hiredis`, `drogon`
- **Databases**: PostgreSQL 14+, Redis 6+
- **Frontend**: Node.js 18+, npm/yarn

> Detailed build instructions will be added as development progresses.

## 🗺️ Development Roadmap

- [ ] **Phase 1**: C++ Foundation & Database Setup (Drogon init, CMake, PostgreSQL, Redis)
- [ ] **Phase 2**: Security & Authentication (JWT, C++ HttpFilters, RBAC)
- [ ] **Phase 3**: Core CRUD Modules (Routes, Stops, Buses APIs)
- [ ] **Phase 4**: High-Performance Tracking Engine (Redis, WebSockets, Async DB Batching)
- [ ] **Phase 5**: Frontend Foundation (React, Tailwind, Auth Flow)
- [ ] **Phase 6**: Admin & Fleet Management UI
- [ ] **Phase 7**: Live Tracking UI (Google Maps, WebSocket Client)
- [ ] **Phase 8**: Passenger Portal (Search, ETA, Notifications)
- [ ] **Phase 9**: Dockerization & Deployment

## 📄 License

This project is licensed under the MIT License.