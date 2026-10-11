/* idb-keyval minimal UMD offline - MIT */
(function(g,f){typeof exports==='object'&&typeof module!=='undefined'?f(exports):typeof define==='function'&&define.amd?define(['exports'],f):(g=typeof globalThis!=='undefined'?globalThis:g||self,f(g.idbKeyval={}));})(this,function(e){
function pr(r){return new Promise(function(res,rej){r.oncomplete=r.onsuccess=function(){res(r.result);}; r.onabort=r.onerror=function(){rej(r.error);};});}
function createStore(dbN,stN){var dbp=new Promise(function(res,rej){var o=indexedDB.open(dbN,1); o.onupgradeneeded=function(){o.result.createObjectStore(stN);}; o.onsuccess=function(){res(o.result);}; o.onerror=function(){rej(o.error);};}); return function(t,cb){return dbp.then(function(db){return cb(db.transaction(stN,t).objectStore(stN));});};}
var def=createStore('keyval-store','keyval');
function get(k,s){s=s||def; return s('readonly',function(o){return pr(o.get(k));});}
function set(k,v,s){s=s||def; return s('readwrite',function(o){o.put(v,k); return pr(o.transaction);});}
function del(k,s){s=s||def; return s('readwrite',function(o){o.delete(k); return pr(o.transaction);});}
function clear(s){s=s||def; return s('readwrite',function(o){o.clear(); return pr(o.transaction);});}
function keys(s){s=s||def; return s('readonly',function(o){if(o.getAllKeys) return pr(o.getAllKeys()); var r=[]; return new Promise(function(res){o.openKeyCursor().onsuccess=function(){var c=this.result; if(!c){res(r);return;} r.push(c.key); c.continue();};});});}
function entries(s){s=s||def; return s('readonly',function(o){var r=[]; return new Promise(function(res){o.openCursor().onsuccess=function(){var c=this.result; if(!c){res(r);return;} r.push([c.key,c.value]); c.continue();};});});}
e.get=get; e.set=set; e.del=del; e.clear=clear; e.keys=keys; e.entries=entries; e.createStore=createStore;
if(typeof window!=='undefined') window.idbKeyval=e;
});
