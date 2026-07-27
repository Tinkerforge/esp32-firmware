module.exports = {
    plugins: [
        require('autoprefixer'),
        require('cssnano') ({
            preset: ['advanced', {
                // postcss-svgo warns on Bootstrap's percent-encoded inline SVG values
                // generated for form switch backgrounds (e.g. `rgb(...%%29`) in newer Sass).
                // Disable only svgo to keep the rest of cssnano optimizations.
                svgo: false,
            }],
        }),
    ],
};
