const TOKEN_KEY = 'busyatri_token';

export const getToken = () => localStorage.getItem(TOKEN_KEY);

export const setToken = (token) => localStorage.setItem(TOKEN_KEY, token);

export const clearToken = () => localStorage.removeItem(TOKEN_KEY);

export const getUserFromToken = () => {
  const token = getToken();
  if (!token) return null;
  
  try {
    const payloadBase64 = token.split('.')[1];
    if (!payloadBase64) return null;

    // JWT payloads use base64url, whereas atob() only accepts regular
    // base64. Convert the alphabet and restore padding before decoding so a
    // Driver/Admin session also survives a page refresh.
    const base64 = payloadBase64.replace(/-/g, '+').replace(/_/g, '/');
    const paddedBase64 = base64.padEnd(Math.ceil(base64.length / 4) * 4, '=');
    const payloadJson = atob(paddedBase64);
    const payload = JSON.parse(payloadJson);

    const roleId = Number(payload.role_id);
    if (!payload.user_id || !Number.isInteger(roleId)) return null;
    
    return {
      userId: payload.user_id,
      roleId,
    };
  } catch (error) {
    console.error('Error parsing token:', error);
    return null;
  }
};

async function request(method, path, body = null) {
  const headers = {
    'Content-Type': 'application/json',
  };

  const token = getToken();
  if (token) {
    headers['Authorization'] = `Bearer ${token}`;
  }

  const options = {
    method,
    headers,
  };

  if (body) {
    options.body = JSON.stringify(body);
  }

  const response = await fetch(path, options);
  
  if (response.status === 401) {
    clearToken();
  }
  
  if (!response.ok) {
    let errorMessage = 'Request failed';
    try {
      const errorData = await response.json();
      errorMessage = errorData.error || errorData.message || errorMessage;
    } catch (e) {
      errorMessage = response.statusText || errorMessage;
    }
    throw new Error(errorMessage);
  }
  
  // Return null/empty string for 204 No Content
  if (response.status === 204) {
    return null;
  }
  
  return await response.json();
}

export const api = {
  get: (path) => request('GET', path),
  post: (path, body) => request('POST', path, body),
  put: (path, body) => request('PUT', path, body),
  del: (path) => request('DELETE', path),
};
