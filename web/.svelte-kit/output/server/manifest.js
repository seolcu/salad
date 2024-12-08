export const manifest = (() => {
function __memo(fn) {
	let value;
	return () => value ??= (value = fn());
}

return {
	appDir: "_app",
	appPath: "_app",
	assets: new Set(["favicon.png"]),
	mimeTypes: {".png":"image/png"},
	_: {
		client: {"start":"_app/immutable/entry/start.BhVzj_UZ.js","app":"_app/immutable/entry/app.DgIFHSlg.js","imports":["_app/immutable/entry/start.BhVzj_UZ.js","_app/immutable/chunks/entry.C2NXMsHX.js","_app/immutable/chunks/runtime.CR0FSWDG.js","_app/immutable/entry/app.DgIFHSlg.js","_app/immutable/chunks/runtime.CR0FSWDG.js","_app/immutable/chunks/store.CBwLL1Js.js","_app/immutable/chunks/disclose-version.Bp6SczzB.js"],"stylesheets":[],"fonts":[],"uses_env_dynamic_public":false},
		nodes: [
			__memo(() => import('./nodes/0.js')),
			__memo(() => import('./nodes/1.js')),
			__memo(() => import('./nodes/2.js'))
		],
		routes: [
			{
				id: "/",
				pattern: /^\/$/,
				params: [],
				page: { layouts: [0,], errors: [1,], leaf: 2 },
				endpoint: null
			}
		],
		matchers: async () => {
			
			return {  };
		},
		server_assets: {}
	}
}
})();
