
const CACHE = "salvation-beads-v11-polished";
const ASSETS = [
  "./",
  "./index.html",
  "./libs/tailwind.css",
  "./libs/Sortable.min.js",
  "./libs/idb-keyval.js",
  "./fonts/fonts.css",
  "./themes/renewal.webp",
  "./themes/galilee.webp",
  "./themes/olive.webp",
  "./themes/heavenly.webp",
  "./themes/starlit.webp",
  "./themes/crimson.webp",
  "./themes/emerald.webp",
  "./themes/sapphire.webp",
  "./themes/crystalClear.webp",
  "./themes/tropicalBloom.webp",
  "./themes/winterBenediction.webp",
  "./themes/mecca_day.webp",
  "./themes/mecca_night.webp"
];
self.addEventListener("install", e => { e.waitUntil(caches.open(CACHE).then(c => c.addAll(ASSETS)).then(()=>self.skipWaiting())); });
self.addEventListener("activate", e => { e.waitUntil(caches.keys().then(keys => Promise.all(keys.filter(k=>k!==CACHE).map(k=>caches.delete(k)))).then(()=>self.clients.claim())); });
self.addEventListener("fetch", e => { if (e.request.url.includes("unsplash.com") || e.request.url.includes("pexels.com") || e.request.url.includes("google")) return; e.respondWith(caches.match(e.request).then(cached => cached || fetch(e.request).catch(()=>cached))); });
