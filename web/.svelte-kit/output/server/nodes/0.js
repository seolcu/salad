

export const index = 0;
let component_cache;
export const component = async () => component_cache ??= (await import('../entries/fallbacks/layout.svelte.js')).default;
export const imports = ["_app/immutable/nodes/0.fRScne08.js","_app/immutable/chunks/disclose-version.Bp6SczzB.js","_app/immutable/chunks/runtime.CR0FSWDG.js"];
export const stylesheets = [];
export const fonts = [];
