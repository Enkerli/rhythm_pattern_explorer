#!/usr/bin/env node

/**
 * Documentation Maintenance Script
 * 
 * A utility script to help maintain consistency between Markdown and HTML documentation,
 * validate mathematical equations, and ensure documentation completeness.
 * 
 * Usage:
 *   node docs-maintenance.js [command]
 * 
 * Commands:
 *   check    - Check documentation completeness and consistency
 *   sync     - Sync changes between Markdown and HTML
 *   validate - Validate mathematical equations
 *   report   - Generate maintenance report
 */

const fs = require('fs');
const path = require('path');

class DocumentationMaintenance {
    constructor() {
        this.markdownPath = './README.md';
        this.htmlPath = './app/documentation.html';
        this.appPath = './app/';
    }

    /**
     * Check if all mathematical equations are documented
     */
    checkEquationDocumentation() {
        console.log('🔍 Checking equation documentation...');
        
        const findings = {
            inApp: [],
            inMarkdown: [],
            inHTML: [],
            missing: []
        };
        
        // Read application files to find equations
        const appFiles = this.getJavaScriptFiles();
        appFiles.forEach(file => {
            const content = fs.readFileSync(file, 'utf8');
            const equations = this.extractEquationsFromCode(content);
            findings.inApp.push(...equations.map(eq => ({ file, equation: eq })));
        });
        
        // Read documentation files
        if (fs.existsSync(this.markdownPath)) {
            const markdownContent = fs.readFileSync(this.markdownPath, 'utf8');
            findings.inMarkdown = this.extractEquationsFromMarkdown(markdownContent);
        }
        
        if (fs.existsSync(this.htmlPath)) {
            const htmlContent = fs.readFileSync(this.htmlPath, 'utf8');
            findings.inHTML = this.extractEquationsFromHTML(htmlContent);
        }
        
        // Find missing equations
        findings.inApp.forEach(({ file, equation }) => {
            const inMarkdown = findings.inMarkdown.some(eq => eq.includes(equation));
            const inHTML = findings.inHTML.some(eq => eq.includes(equation));
            
            if (!inMarkdown || !inHTML) {
                findings.missing.push({ file, equation, inMarkdown, inHTML });
            }
        });
        
        return findings;
    }
    
    /**
     * Get all JavaScript files in the app directory
     */
    getJavaScriptFiles() {
        const files = [];
        const appDir = path.resolve(this.appPath);
        
        if (fs.existsSync(appDir)) {
            const dirFiles = fs.readdirSync(appDir);
            dirFiles.forEach(file => {
                if (file.endsWith('.js')) {
                    files.push(path.join(appDir, file));
                }
            });
        }
        
        return files;
    }
    
    /**
     * Extract mathematical equations from JavaScript code
     */
    extractEquationsFromCode(content) {
        const equations = [];
        
        // Look for LaTeX patterns
        const latexPatterns = [
            /\$\$([^$]+)\$\$/g,          // Display math
            /\$([^$]+)\$/g,              // Inline math
            /\\sum[^}]*}/g,              // Summation
            /\\frac{[^}]*}{[^}]*}/g,     // Fractions
            /e\^{[^}]*}/g,               // Exponentials
            /\\arctan2/g,                // Arc tangent
            /\\pi/g,                     // Pi
            /\\text{[^}]*}/g             // Text in math
        ];
        
        latexPatterns.forEach(pattern => {
            const matches = content.match(pattern);
            if (matches) {
                equations.push(...matches);
            }
        });
        
        return [...new Set(equations)]; // Remove duplicates
    }
    
    /**
     * Extract equations from Markdown content
     */
    extractEquationsFromMarkdown(content) {
        const equations = [];
        
        // Look for code blocks with equations
        const codeBlockPattern = /```\s*([^`]+)\s*```/g;
        const mathBlockPattern = /\$\$([^$]+)\$\$/g;
        
        let match;
        while ((match = codeBlockPattern.exec(content)) !== null) {
            if (match[1].includes('∑') || match[1].includes('e^') || match[1].includes('π')) {
                equations.push(match[1].trim());
            }
        }
        
        while ((match = mathBlockPattern.exec(content)) !== null) {
            equations.push(match[1].trim());
        }
        
        return equations;
    }
    
    /**
     * Extract equations from HTML content
     */
    extractEquationsFromHTML(content) {
        const equations = [];
        
        // Look for MathJax expressions
        const mathPatterns = [
            /\$\$([^$]+)\$\$/g,
            /\$([^$]+)\$/g
        ];
        
        mathPatterns.forEach(pattern => {
            let match;
            while ((match = pattern.exec(content)) !== null) {
                equations.push(match[1].trim());
            }
        });
        
        return equations;
    }
    
    /**
     * Generate maintenance report
     */
    generateReport() {
        console.log('📊 Generating documentation maintenance report...');
        
        const report = {
            timestamp: new Date().toISOString(),
            files: {
                markdown: fs.existsSync(this.markdownPath),
                html: fs.existsSync(this.htmlPath),
                app: this.getJavaScriptFiles().length
            },
            equations: this.checkEquationDocumentation(),
            recommendations: []
        };
        
        // Generate recommendations
        if (report.equations.missing.length > 0) {
            report.recommendations.push({
                type: 'missing-equations',
                count: report.equations.missing.length,
                message: 'Some equations found in application code are not documented'
            });
        }
        
        if (!report.files.html && report.files.markdown) {
            report.recommendations.push({
                type: 'missing-html',
                message: 'HTML documentation not found, consider creating it'
            });
        }
        
        return report;
    }
    
    /**
     * Print formatted report
     */
    printReport(report) {
        console.log('\n📋 DOCUMENTATION MAINTENANCE REPORT');
        console.log('=====================================');
        console.log(`Generated: ${report.timestamp}`);
        console.log(`\n📁 Files:`);
        console.log(`  Markdown: ${report.files.markdown ? '✅' : '❌'}`);
        console.log(`  HTML: ${report.files.html ? '✅' : '❌'}`);
        console.log(`  App JS files: ${report.files.app}`);
        
        console.log(`\n🧮 Equations:`);
        console.log(`  In app code: ${report.equations.inApp.length}`);
        console.log(`  In markdown: ${report.equations.inMarkdown.length}`);
        console.log(`  In HTML: ${report.equations.inHTML.length}`);
        console.log(`  Missing from docs: ${report.equations.missing.length}`);
        
        if (report.equations.missing.length > 0) {
            console.log('\n❌ Missing equations:');
            report.equations.missing.forEach(({ file, equation }) => {
                console.log(`  ${path.basename(file)}: ${equation.substring(0, 50)}...`);
            });
        }
        
        if (report.recommendations.length > 0) {
            console.log('\n💡 Recommendations:');
            report.recommendations.forEach(rec => {
                console.log(`  • ${rec.message}`);
            });
        }
        
        console.log('\n✅ Report complete');
    }
    
    /**
     * Main execution function
     */
    run(command = 'check') {
        console.log('📚 Documentation Maintenance Tool');
        console.log('==================================\n');
        
        switch (command) {
            case 'check':
            case 'report':
                const report = this.generateReport();
                this.printReport(report);
                break;
                
            case 'validate':
                console.log('🔍 Validating mathematical equations...');
                const equations = this.checkEquationDocumentation();
                console.log(`Found ${equations.inApp.length} equations in application code`);
                console.log(`Found ${equations.missing.length} missing from documentation`);
                break;
                
            default:
                console.log('Available commands: check, validate, report');
                console.log('Usage: node docs-maintenance.js [command]');
        }
    }
}

// Run if called directly
if (require.main === module) {
    const command = process.argv[2] || 'check';
    const maintenance = new DocumentationMaintenance();
    maintenance.run(command);
}

module.exports = DocumentationMaintenance;