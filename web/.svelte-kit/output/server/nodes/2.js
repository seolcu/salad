

export const index = 2;
let component_cache;
export const component = async () => component_cache ??= (await import('../entries/pages/_page.svelte.js')).default;
export const imports = ["_app/immutable/nodes/2.CkAq-CCh.js","_app/immutable/chunks/disclose-version.Bp6SczzB.js","_app/immutable/chunks/runtime.CR0FSWDG.js","_app/immutable/chunks/legacy.B_hBjxRD.js"];
export const stylesheets = [];
export const fonts = [];
