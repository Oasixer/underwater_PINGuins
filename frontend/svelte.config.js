import adapter from '@sveltejs/adapter-static';
// import preprocess from 'svelte-preprocess';
// import sveltePreprocess from 'svelte-preprocess';
import { vitePreprocess } from '@sveltejs/kit/vite';

/** @type {import('@sveltejs/kit').Config} */
const config = {
	// Consult https://github.com/sveltejs/svelte-preprocess
	// for more information about preprocessors
	// preprocess: preprocess(),
	preprocess: vitePreprocess({ postcss: true }),
	// sveltePreprocess({
	//   sourceMap: !production,
	//   postcss: {
	//     plugins: [
	//      require("tailwindcss"),
	//      require("autoprefixer"),
	//     ],
	//   },
	// }),
	onwarn: (warning, handler) => {
		if (warning.code.startsWith('a11y-')) return;
		handler(warning);
	},

	kit: {
		adapter: adapter({
			fallback: 'index.html'
		})
	}
};

export default config;
// import adapter from '@sveltejs/adapter-auto';
// import { vitePreprocess } from '@sveltejs/kit/vite';
// // import { preprocess } from 'svelte-preprocess'
//
// /** @type {import('@sveltejs/kit').Config} */
// const config = {
// 	// Consult https://kit.svelte.dev/docs/integrations#preprocessors
// 	// for more information about preprocessors
// 	preprocess: vitePreprocess({
// 		'postcss': true,
// 	}),
// 	trailingSlash: 'always',
// 	// preprocess: vitePreprocess(),
//
// 	kit: {
// 		// adapter-auto only supports some environments, see https://kit.svelte.dev/docs/adapter-auto for a list.
// 		// If your environment is not supported or you settled on a specific environment, switch out the adapter.
// 		// See https://kit.svelte.dev/docs/adapters for more information about adapters.
// 		adapter: adapter(
// 			{
// 				fallback: 'app.html',
// 			}
// 		)
// 	}
// };
//
// export default config;
