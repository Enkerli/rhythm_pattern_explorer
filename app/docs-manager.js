// === DOCUMENTATION MANAGER MODULE ===
/**
 * Documentation Management System
 * 
 * Provides utilities for maintaining consistency between Markdown and HTML documentation,
 * managing mathematical equation rendering, and ensuring documentation stays up-to-date
 * with application features.
 * 
 * Key Features:
 * - Dual documentation format management (Markdown + HTML)
 * - Mathematical equation consistency checking
 * - Documentation content validation
 * - Cross-reference verification
 * - Feature documentation completeness tracking
 */

class DocumentationManager {
    /**
     * Initialize the Documentation Manager
     */
    constructor() {
        this.features = new Map();
        this.equations = new Map();
        this.algorithms = new Map();
        
        this.initializeFeatureRegistry();
        this.initializeEquationRegistry();
        
        console.log('📚 Documentation Manager initialized');
    }
    
    /**
     * Initialize registry of application features that need documentation
     */
    initializeFeatureRegistry() {
        // Core features that should be documented
        const coreFeatures = [
            'universal-pattern-input',
            'perfect-balance-analysis',
            'center-of-gravity-calculation',
            'barlow-transformer',
            'euclidean-transformer',
            'progressive-offsets',
            'rhythm-mutator',
            'funky-generator',
            'pattern-database',
            'systematic-explorer',
            'interactive-sequencer'
        ];
        
        coreFeatures.forEach(feature => {
            this.features.set(feature, {
                name: feature,
                documented: false,
                hasEquations: false,
                lastUpdated: null
            });
        });
    }
    
    /**
     * Initialize registry of mathematical equations used in the application
     */
    initializeEquationRegistry() {
        // Core mathematical equations
        const equations = [
            {
                id: 'milne-perfect-balance',
                latex: '\\sum_{j=1}^{k} e^{i2\\pi k_j/n} = 0',
                description: "Milne's Perfect Balance Condition",
                context: 'perfect-balance-analysis'
            },
            {
                id: 'balance-magnitude',
                latex: '\\left|\\sum_{j=1}^{k} e^{i2\\pi k_j/n}\\right|',
                description: 'Balance Magnitude Calculation',
                context: 'perfect-balance-analysis'
            },
            {
                id: 'normalized-balance',
                latex: '\\frac{\\left|\\sum_{j=1}^{k} e^{i2\\pi k_j/n}\\right|}{k}',
                description: 'Normalized Balance Score',
                context: 'perfect-balance-analysis'
            },
            {
                id: 'center-of-gravity',
                latex: 'CoG = \\frac{1}{k}\\sum_{j=1}^{k} e^{i2\\pi k_j/n}',
                description: 'Center of Gravity Calculation',
                context: 'center-of-gravity-analysis'
            },
            {
                id: 'cog-angle',
                latex: '\\theta = \\arctan2(\\text{Im}(CoG), \\text{Re}(CoG)) \\times \\frac{180}{\\pi}',
                description: 'Center of Gravity Angular Position',
                context: 'center-of-gravity-analysis'
            },
            {
                id: 'barlow-indispensability',
                latex: 'I(p, n) = \\begin{cases} 1.0 & \\text{if } p = 0 \\\\ 0.75 & \\text{if } p = n-1 \\\\ \\frac{1}{\\prod_{q \\text{ prime}} q^{v_q(n/\\gcd(p,n))}} & \\text{otherwise} \\end{cases}',
                description: 'Barlow Indispensability Calculation',
                context: 'barlow-transformer'
            },
            {
                id: 'euclidean-rhythm',
                latex: 'E(k, n) = \\text{Bjorklund}(k, n-k)',
                description: 'Euclidean Rhythm Generation',
                context: 'euclidean-transformer'
            },
            {
                id: 'pattern-rotation',
                latex: '\\text{Rotate}(P, r) = \\{(p + r) \\bmod n : p \\in P\\}',
                description: 'Pattern Rotation Formula',
                context: 'progressive-offsets'
            }
        ];
        
        equations.forEach(eq => {
            this.equations.set(eq.id, eq);
        });
    }
    
    /**
     * Validate that all documented equations are properly rendered
     * @returns {Object} Validation results
     */
    validateEquationRendering() {
        const results = {
            total: this.equations.size,
            rendered: 0,
            missing: [],
            errors: []
        };
        
        this.equations.forEach((equation, id) => {
            // Check if equation exists in documentation
            const docElement = document.querySelector(`[data-equation-id="${id}"]`);
            if (docElement) {
                results.rendered++;
                
                // Check if MathJax has processed it
                const mathJaxElement = docElement.querySelector('.MathJax');
                if (!mathJaxElement) {
                    results.errors.push(`Equation ${id} not processed by MathJax`);
                }
            } else {
                results.missing.push(id);
            }
        });
        
        return results;
    }
    
    /**
     * Generate LaTeX code for an equation
     * @param {string} equationId - ID of the equation
     * @param {boolean} displayMode - Whether to use display mode
     * @returns {string} LaTeX markup
     */
    generateEquationLatex(equationId, displayMode = true) {
        const equation = this.equations.get(equationId);
        if (!equation) {
            console.warn(`Documentation: Unknown equation ID: ${equationId}`);
            return '';
        }
        
        const wrapper = displayMode ? '$$' : '$';
        return `${wrapper}${equation.latex}${wrapper}`;
    }
    
    /**
     * Generate HTML for an equation with proper metadata
     * @param {string} equationId - ID of the equation
     * @param {Object} options - Rendering options
     * @returns {string} HTML markup
     */
    generateEquationHTML(equationId, options = {}) {
        const equation = this.equations.get(equationId);
        if (!equation) {
            console.warn(`Documentation: Unknown equation ID: ${equationId}`);
            return '';
        }
        
        const {
            displayMode = true,
            showDescription = true,
            className = 'equation-container'
        } = options;
        
        const latexCode = this.generateEquationLatex(equationId, displayMode);
        
        return `
            <div class="${className}" data-equation-id="${equationId}">
                <div class="equation-title">${equation.description}</div>
                <div class="equation-display">${latexCode}</div>
                ${showDescription ? `<div class="equation-description">${equation.description}</div>` : ''}
            </div>
        `;
    }
    
    /**
     * Get all equations for a specific context
     * @param {string} context - Context name (e.g., 'perfect-balance-analysis')
     * @returns {Array} Array of equations for the context
     */
    getEquationsForContext(context) {
        const contextEquations = [];
        this.equations.forEach((equation, id) => {
            if (equation.context === context) {
                contextEquations.push({ id, ...equation });
            }
        });
        return contextEquations;
    }
    
    /**
     * Generate documentation section for a feature
     * @param {string} featureName - Name of the feature
     * @param {Object} content - Content configuration
     * @returns {string} Generated HTML
     */
    generateFeatureDocumentation(featureName, content) {
        const feature = this.features.get(featureName);
        if (!feature) {
            console.warn(`Documentation: Unknown feature: ${featureName}`);
            return '';
        }
        
        let html = `<section id="${featureName}" class="docs-section">`;
        html += `<h2>${content.title || featureName}</h2>`;
        
        if (content.description) {
            html += `<p>${content.description}</p>`;
        }
        
        // Add equations if any
        const equations = this.getEquationsForContext(featureName);
        equations.forEach(eq => {
            html += this.generateEquationHTML(eq.id);
        });
        
        // Add examples if any
        if (content.examples) {
            html += '<h3>Examples</h3>';
            content.examples.forEach(example => {
                html += `<div class="code-example">${example}</div>`;
            });
        }
        
        html += '</section>';
        return html;
    }
    
    /**
     * Check documentation completeness
     * @returns {Object} Completeness report
     */
    checkDocumentationCompleteness() {
        const report = {
            features: {
                total: this.features.size,
                documented: 0,
                missing: []
            },
            equations: {
                total: this.equations.size,
                implemented: 0,
                missing: []
            }
        };
        
        // Check feature documentation
        this.features.forEach((feature, name) => {
            if (feature.documented) {
                report.features.documented++;
            } else {
                report.features.missing.push(name);
            }
        });
        
        // Check equation implementation
        this.equations.forEach((equation, id) => {
            // This would check if the equation is actually used in the app
            // For now, assume all registered equations are implemented
            report.equations.implemented++;
        });
        
        return report;
    }
    
    /**
     * Generate table of contents for documentation
     * @returns {string} HTML table of contents
     */
    generateTableOfContents() {
        let toc = '<nav class="docs-nav"><h3>📖 Quick Navigation</h3><ul>';
        
        const sections = [
            { id: 'overview', title: 'Overview' },
            { id: 'getting-started', title: 'Getting Started' },
            { id: 'mathematical-theory', title: 'Mathematical Theory' },
            { id: 'algorithms', title: 'Core Algorithms' },
            { id: 'pattern-input', title: 'Pattern Input System' },
            { id: 'transformers', title: 'Pattern Transformers' },
            { id: 'analysis-engine', title: 'Analysis Engine' },
            { id: 'generators', title: 'Pattern Generators' },
            { id: 'technical-architecture', title: 'Technical Architecture' },
            { id: 'examples', title: 'Usage Examples' }
        ];
        
        sections.forEach(section => {
            toc += `<li><a href="#${section.id}">${section.title}</a></li>`;
        });
        
        toc += '</ul></nav>';
        return toc;
    }
    
    /**
     * Sync equation changes between HTML and Markdown documentation
     * @param {string} equationId - ID of equation to sync
     * @param {string} newLatex - Updated LaTeX code
     */
    syncEquationChange(equationId, newLatex) {
        const equation = this.equations.get(equationId);
        if (equation) {
            equation.latex = newLatex;
            equation.lastUpdated = new Date().toISOString();
            
            console.log(`📚 Documentation: Updated equation ${equationId}`);
            
            // TODO: Implement actual sync to Markdown and HTML files
            // This would update both documentation formats automatically
        }
    }
    
    /**
     * Generate documentation maintenance report
     * @returns {Object} Maintenance report
     */
    generateMaintenanceReport() {
        const completeness = this.checkDocumentationCompleteness();
        const equationValidation = this.validateEquationRendering();
        
        return {
            timestamp: new Date().toISOString(),
            completeness,
            equations: equationValidation,
            recommendations: this.generateRecommendations(completeness, equationValidation)
        };
    }
    
    /**
     * Generate maintenance recommendations
     * @param {Object} completeness - Completeness report
     * @param {Object} equationValidation - Equation validation results
     * @returns {Array} Array of recommendations
     */
    generateRecommendations(completeness, equationValidation) {
        const recommendations = [];
        
        if (completeness.features.missing.length > 0) {
            recommendations.push({
                type: 'feature-documentation',
                priority: 'high',
                message: `${completeness.features.missing.length} features need documentation`,
                features: completeness.features.missing
            });
        }
        
        if (equationValidation.missing.length > 0) {
            recommendations.push({
                type: 'missing-equations',
                priority: 'medium',
                message: `${equationValidation.missing.length} equations not found in documentation`,
                equations: equationValidation.missing
            });
        }
        
        if (equationValidation.errors.length > 0) {
            recommendations.push({
                type: 'rendering-errors',
                priority: 'high',
                message: `${equationValidation.errors.length} equation rendering errors`,
                errors: equationValidation.errors
            });
        }
        
        return recommendations;
    }
}

// Create and export global documentation manager instance
const docsManager = new DocumentationManager();

// Export for use in other modules
if (typeof module !== 'undefined' && module.exports) {
    module.exports = { DocumentationManager, docsManager };
}