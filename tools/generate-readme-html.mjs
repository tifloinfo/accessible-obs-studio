import fs from "node:fs";

const markedModule = await import(process.env.MARKED_MODULE ?? "marked");
const { marked } = markedModule;

const documents = [
  ["README.md", "docs/README.en-US.html", "en", "Skip to content"],
  ["docs/README.de-DE.md", "docs/README.de-DE.html", "de", "Zum Inhalt springen"],
  ["docs/README.es-ES.md", "docs/README.es-ES.html", "es", "Saltar al contenido"],
  ["docs/README.fr-FR.md", "docs/README.fr-FR.html", "fr", "Aller au contenu"],
  ["docs/README.ru-RU.md", "docs/README.ru-RU.html", "ru", "Перейти к содержимому"],
  ["docs/README.uk-UA.md", "docs/README.uk-UA.html", "uk", "Перейти до вмісту"],
];

const css = `
  :root { color-scheme: light dark; }
  * { box-sizing: border-box; }
  body {
    margin: 0;
    font-family: system-ui, -apple-system, "Segoe UI", sans-serif;
    font-size: 1rem;
    line-height: 1.6;
    background: Canvas;
    color: CanvasText;
  }
  main { max-width: 72rem; margin: 0 auto; padding: 1.5rem; }
  h1, h2, h3 { line-height: 1.25; }
  a { color: LinkText; text-decoration: underline; text-underline-offset: .15em; }
  a:focus-visible { outline: 3px solid Highlight; outline-offset: 3px; }
  .skip-link {
    position: absolute;
    left: .5rem;
    top: -5rem;
    padding: .75rem;
    background: Canvas;
    color: CanvasText;
    border: 2px solid CanvasText;
  }
  .skip-link:focus { top: .5rem; }
  code { overflow-wrap: anywhere; }
  img { max-width: 100%; height: auto; }
  @media (prefers-reduced-motion: reduce) {
    * { scroll-behavior: auto !important; }
  }
`;

function escapeHtml(value) {
  return value
    .replaceAll("&", "&amp;")
    .replaceAll("<", "&lt;")
    .replaceAll(">", "&gt;")
    .replaceAll('"', "&quot;");
}

for (const [sourcePath, destinationPath, language, skipText] of documents) {
  const source = fs.readFileSync(sourcePath, "utf8");
  let body = marked.parse(source, { gfm: true });
  body = body.replaceAll('src="assets/', 'src="../assets/');

  const titleMatch = source.match(/^#\s+(.+)$/m);
  const title = titleMatch?.[1] ?? "Accessible OBS Studio 1.0";
  const html = `<!doctype html>
<html lang="${language}">
<head>
<meta charset="utf-8">
<meta name="viewport" content="width=device-width, initial-scale=1">
<title>${escapeHtml(title)}</title>
<style>${css}</style>
</head>
<body>
<a class="skip-link" href="#main-content">${escapeHtml(skipText)}</a>
<main id="main-content">
${body}</main>
</body>
</html>
`;

  fs.writeFileSync(destinationPath, html, "utf8");
  console.log(destinationPath);
}
