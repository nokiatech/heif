/**
 * Cross-Origin Isolation — page-side registration
 *
 * Registers coi-sw.js (one directory up from js/) and triggers a
 * single reload so the service worker can intercept the document
 * fetch and inject the COEP + COOP headers.
 *
 * Implements COEP degradation: if require-corp fails on first load,
 * falls back to credentialless mode on the next reload.
 *
 * Session-storage flags with distinct values prevent reload loops.
 */
(function () {
  var RELOAD_KEY = 'coi-sw-reloaded';
  var COEP_FAIL  = 'coi-coep-failed';
  var reloaded   = sessionStorage.getItem(RELOAD_KEY);
  sessionStorage.removeItem(RELOAD_KEY);
  var coepDegrading = (reloaded === 'coepdegrade');

  /* If the page was previously served by the SW but isolation failed,
     record that require-corp is broken in this environment. */
  if (navigator.serviceWorker && navigator.serviceWorker.controller
      && !window.crossOriginIsolated) {
    sessionStorage.setItem(COEP_FAIL, '1');
  }
  var coepHasFailed = sessionStorage.getItem(COEP_FAIL);

  /* If a controller exists, send it the right COEP mode. */
  if (navigator.serviceWorker && navigator.serviceWorker.controller) {
    var reloadToDegrade = !(coepDegrading || window.crossOriginIsolated);
    navigator.serviceWorker.controller.postMessage({
      type:  'coepCredentialless',
      value: !!(coepDegrading || coepHasFailed)
    });
    if (reloadToDegrade) {
      sessionStorage.setItem(RELOAD_KEY, 'coepdegrade');
      location.reload();
      return;
    }
  }

  /* Already isolated (real server headers or SW already active).
     Uses !== false so undefined (unsupported browsers) also exits. */
  if (window.crossOriginIsolated !== false) return;

  /* Don't re-register after a reload we triggered ourselves. */
  if (reloaded) return;

  /* Secure context required for service workers (HTTPS or localhost). */
  if (!window.isSecureContext) return;
  if (!navigator.serviceWorker) return;

  navigator.serviceWorker
    .register('coi-sw.js', { scope: './' })
    .then(function (reg) {
      /* SW just installed or updated — reload to let it intercept. */
      reg.addEventListener('updatefound', function () {
        /* On first install, the 'active && !controller' branch below
           or the controllerchange listener handles the reload.
           Only reload here on genuine updates (new SW replacing
           an existing one). */
        if (navigator.serviceWorker.controller) {
          sessionStorage.setItem(RELOAD_KEY, 'updatefound');
          location.reload();
        }
      });

      /* SW is active but not yet controlling this page (first install). */
      if (reg.active && !navigator.serviceWorker.controller) {
        sessionStorage.setItem(RELOAD_KEY, 'notcontrolling');
        location.reload();
      }
    })
    .catch(function (err) {
      console.error('[coi-register] SW registration failed:', err);
    });

  /* Handle first-install race: if the SW activates and calls
     clients.claim() after the register() callback already ran,
     the reg.active check above would have missed it.  This
     catches that case. */
  navigator.serviceWorker.addEventListener('controllerchange', function () {
    if (!sessionStorage.getItem(RELOAD_KEY)) {
      sessionStorage.setItem(RELOAD_KEY, 'controllerchange');
      location.reload();
    }
  });
})();
