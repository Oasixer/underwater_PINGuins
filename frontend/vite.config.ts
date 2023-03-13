// vite.config.js

import { sveltekit } from '@sveltejs/kit/vite';
import { defineConfig } from 'vite';

/** @type {import('vite').UserConfig} */
export default ({ mode }) => {
	let devEnvSettings = {};
	if (mode === 'development') {
		devEnvSettings = {
			server: {
				port: 3000,
				proxy: {
					'/api': 'http://localhost:8080'
				}
			},
		}
	}
	return defineConfig({
		plugins: [sveltekit()],
		...devEnvSettings,

	});
};// import { sveltekit } from '@sveltejs/kit/vite';
// // import {svelte} from "@sveltejs/vite-plugin-svelte"
// import { defineConfig } from 'vite';

// export default defineConfig({
// 	base: "/",
// 	plugins:  [svelte()],
// 	plugins: [sveltekit()]
// 	build: {
// 		outDir: "../maurice/website_compiled",
// 	},
// 	optimizeDeps: {
//         exclude: ["svelte-navigator"],
//     },
// 	server: {
//         host: "127.0.0.1",
//         port: 6699,
//     },
// 	define: {
//         "import.meta.vitest": false,
//     },
// });
