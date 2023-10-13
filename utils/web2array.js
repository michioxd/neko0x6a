const fs = require('fs');
const path = require('path');
const minify = require('html-minifier').minify;

const sourceDir = 'www';
const headerFileName = 'src/components/www.h';

function minifyHtml(htmlContent) {
    return minify(htmlContent, {
        collapseWhitespace: true,
        removeComments: true,
        minifyJS: true,
        minifyCSS: true,
        minifyURLs: true,
        minifyHtml: true
    });
}

function getFileName(filePath) {
    const pathArray = filePath.split(/[\\/]/);
    const fileNameWithExtension = pathArray[pathArray.length - 1];
    return fileNameWithExtension;
}

function fn(fileName) {
    return fileName.replace(/\../g, (match) => match.charAt(1).toUpperCase());
}

function convertHex(inputString, fileName) {
    return `const char ${fn(fileName)}[] PROGMEM = {${Array.from(minifyHtml(inputString))
        .map((char) => `0x${char.charCodeAt(0).toString(16)}`)
        .join(', ')}, 0x00};`;
}

const files = fs.readdirSync(sourceDir);

fs.writeFileSync(
    headerFileName,
    `#include <Arduino.h>\n\n${files
        .filter((file) => path.extname(file).toLowerCase() === '.html')
        .map((file) => {
            const filePath = path.join(sourceDir, file);
            const content = fs.readFileSync(filePath, 'utf8');
            return convertHex(content, getFileName(filePath));
        })
        .join('\n\n')}`
);

console.log(`OK`);
