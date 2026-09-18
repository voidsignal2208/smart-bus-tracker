ALTER TABLE buses ADD COLUMN IF NOT EXISTS rating DECIMAL(3, 2);
ALTER TABLE buses ADD COLUMN IF NOT EXISTS feedback_score DECIMAL(3, 2);

CREATE TABLE IF NOT EXISTS bus_ratings (
    id UUID DEFAULT gen_random_uuid() PRIMARY KEY,
    bus_id UUID REFERENCES buses(id) ON DELETE CASCADE,
    rating INT NOT NULL CHECK (rating BETWEEN 1 AND 5),
    comment TEXT,
    created_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP
);

CREATE TABLE IF NOT EXISTS bus_feedback (
    id UUID DEFAULT gen_random_uuid() PRIMARY KEY,
    bus_id UUID REFERENCES buses(id) ON DELETE CASCADE,
    is_crowded BOOLEAN DEFAULT FALSE,
    ac_working BOOLEAN DEFAULT TRUE,
    cleanliness INT NOT NULL CHECK (cleanliness BETWEEN 1 AND 5),
    comment TEXT,
    created_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP
);
