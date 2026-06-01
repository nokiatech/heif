/**
 * Cross-Origin Isolation Service Worker
 *
 * Injects Cross-Origin-Embedder-Policy and Cross-Origin-Opener-Policy
 * headers into every response so that the page becomes cross-origin
 * isolated.  This enables SharedArrayBuffer (required by the VVC
 * decoder's pthread workers) on static hosts like GitHub Pages that
 * do not allow custom HTTP headers.
 *
 * Supports runtime switching between require-corp and credentialless
 * COEP modes via postMessage, and programmatic deregistration.
 *
 * The worker is stateless — it caches nothing and stores nothing.
 */

let coepCredentialless = false;

/* Activate immediately, don't wait for existing clients to close. */
self.addEventListener('install', () => self.skipWaiting());

/* Claim all open tabs so the next fetch is already intercepted. */
self.addEventListener('activate', e => e.waitUntil(self.clients.claim()));

/* Accept runtime configuration from the page. */
self.addEventListener('message', ev => {
  if (!ev.data) return;
  if (ev.data.type === 'deregister') {
    self.registration.unregister()
      .then(() => self.clients.matchAll())
      .then(clients => clients.forEach(c => c.navigate(c.url)));
  } else if (ev.data.type === 'coepCredentialless') {
    coepCredentialless = ev.data.value;
  }
});

self.addEventListener('fetch', e => {
  const req = e.request;

  /* Chrome throws TypeError on only-if-cached + non-same-origin mode. */
  if (req.cache === 'only-if-cached' && req.mode !== 'same-origin') return;

  /* In credentialless mode, strip credentials from no-cors requests
     to avoid CORS errors on cross-origin resources. */
  const request = (coepCredentialless && req.mode === 'no-cors')
    ? new Request(req, { credentials: 'omit' })
    : req;

  e.respondWith(
    fetch(request)
      .then(res => {
        /* Opaque responses (status 0) can't have their headers rewritten. */
        if (res.status === 0) return res;

        const headers = new Headers(res.headers);
        headers.set('Cross-Origin-Embedder-Policy',
          coepCredentialless ? 'credentialless' : 'require-corp');
        if (!coepCredentialless) {
          headers.set('Cross-Origin-Resource-Policy', 'cross-origin');
        }
        headers.set('Cross-Origin-Opener-Policy', 'same-origin');

        return new Response(res.body, {
          status:     res.status,
          statusText: res.statusText,
          headers
        });
      })
      .catch(err => {
        console.error('[coi-sw] fetch failed:', err);
        /* Let the browser handle navigation failures natively. */
        if (req.mode === 'navigate') throw err;
        return Response.error();
      })
  );
});
