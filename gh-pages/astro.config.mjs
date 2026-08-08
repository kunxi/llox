import { defineConfig } from "astro/config";
import remarkSmartypants from "remark-smartypants";

export default defineConfig({
  site: "https://kunxi.github.io/llox",
  base: "/llox/",

  markdown: {
    // Shiki syntax highlighting (built-in, no extra package needed)
    shikiConfig: {
      theme: "github-light",
      wrap: true,
    },
    remarkPlugins: [remarkSmartypants],
    // ponytail: rehype plugins only when needed (e.g. rehype-slug for heading links)
  },
});
