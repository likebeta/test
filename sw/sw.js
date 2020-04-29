self.addEventListener('fetch', function (e) {
    console.log('----fetch-----', e.request.url);
    if (e.request.url.endsWith("/1.jpg")) {
        let resp = Response.redirect("https://cdn.ixxoo.me/avatar/5.jpg", 302);
        e.respondWith(resp);
    }
})
