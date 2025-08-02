#!/usr/bin/env python3
"""
Rhythm Pattern Explorer Icon Generator
Creates all iOS icon sizes from an accented quintillo pattern design
Requires: pip install cairosvg pillow
"""

import os
import math
from PIL import Image, ImageDraw, ImageFilter
import cairosvg
from io import BytesIO

def create_base_svg(size=1024):
    """Create SVG for accented quintillo E(5,8) with {10} accent pattern"""
    # Quintillo pattern: E(5,8) = [1,0,1,1,0,1,0,0] 
    # Accent pattern: {10} = [1,0] (accent on onsets 0 and 2)
    
    pattern = [1,0,1,1,0,1,0,0]  # E(5,8) quintillo
    accents = [1,0]  # {10} accent pattern
    
    # Colors
    bg_gradient_start = "#2d2d2d"
    bg_gradient_end = "#1a1a1a"
    inactive_color = "#333333"
    active_color = "#4CAF50"
    accent_color = "#ff4444"
    
    # Dimensions
    center = size // 2
    radius = int(size * 0.35)  # Circle radius
    dot_radius = int(size * 0.024)  # Step dot radius
    
    svg_content = f'''<?xml version="1.0" encoding="UTF-8"?>
<svg width="{size}" height="{size}" viewBox="0 0 {size} {size}" xmlns="http://www.w3.org/2000/svg">
    <defs>
        <linearGradient id="bgGradient" x1="0%" y1="0%" x2="100%" y2="100%">
            <stop offset="0%" style="stop-color:{bg_gradient_start};stop-opacity:1" />
            <stop offset="100%" style="stop-color:{bg_gradient_end};stop-opacity:1" />
        </linearGradient>
        <filter id="glow" x="-50%" y="-50%" width="200%" height="200%">
            <feGaussianBlur stdDeviation="3" result="coloredBlur"/>
            <feMerge> 
                <feMergeNode in="coloredBlur"/>
                <feMergeNode in="SourceGraphic"/>
            </feMerge>
        </filter>
    </defs>
    
    <!-- Background with rounded corners -->
    <rect width="{size}" height="{size}" rx="{size//6}" ry="{size//6}" fill="url(#bgGradient)"/>
    
    <!-- Pattern steps -->'''
    
    onset_counter = 0
    for i in range(8):
        # Calculate position (start at top, go clockwise)
        angle = (i * 45 - 90) * math.pi / 180
        x = center + radius * math.cos(angle)
        y = center + radius * math.sin(angle)
        
        if pattern[i]:  # Active step (onset)
            # Check if this onset should be accented
            is_accented = accents[onset_counter % len(accents)]
            color = accent_color if is_accented else active_color
            glow = 'filter="url(#glow)"' if is_accented else ''
            
            svg_content += f'''
    <circle cx="{x}" cy="{y}" r="{dot_radius}" fill="{color}" {glow}/>'''
            onset_counter += 1
        else:  # Inactive step
            svg_content += f'''
    <circle cx="{x}" cy="{y}" r="{dot_radius}" fill="{inactive_color}" stroke="#555" stroke-width="1"/>'''
    
    svg_content += '''
</svg>'''
    
    return svg_content

def svg_to_png(svg_content, output_size):
    """Convert SVG to PNG at specified size"""
    png_bytes = cairosvg.svg2png(bytestring=svg_content.encode('utf-8'), 
                                output_width=output_size, 
                                output_height=output_size)
    return Image.open(BytesIO(png_bytes))

def generate_all_icons():
    """Generate all required iOS icon sizes"""
    
    # Icon size mappings (filename -> size in pixels)
    icon_sizes = {
        'Icon-AppStore-1024.png': 1024,    # App Store
        'Icon-@3x.png': 180,               # iPhone 60@3x  
        'Icon-60@2x.png': 120,             # iPhone 60@2x
        'Icon-83.5@2x.png': 167,           # iPad Pro 83.5@2x
        'Icon-76@2x.png': 152,             # iPad 76@2x
        'Icon-76.png': 76,                 # iPad 76@1x
        'Icon-Spotlight-40@3x.png': 120,   # iPhone 40@3x
        'Icon-Spotlight-40@2x.png': 80,    # iPhone 40@2x
        'Icon-Spotlight-40@2x-1.png': 80,  # iPad 40@2x
        'Icon-Spotlight-40.png': 40,       # iPad 40@1x
        'Icon-Notification-20@3x.png': 60, # iPhone 20@3x
        'Icon-Notification-20@2x.png': 40, # iPhone 20@2x
        'Icon-Notifications-20@2x.png': 40,# iPad 20@2x
        'Icon-Notifications-20.png': 20,   # iPad 20@1x
        'Icon-29@3x.png': 87,              # iPhone 29@3x
        'Icon-29@2x.png': 58,              # iPhone 29@2x
        'Icon-29.png': 29,                 # iPhone 29@1x
        'Icon-Small@2x-1.png': 58,         # iPad 29@2x
        'Icon-Small-1.png': 29,            # iPad 29@1x
    }
    
    # Output directory
    output_dir = "/Users/alex/Documents/Coding/rhythm_pattern_explorer/RhythmPatternExplorer_iPad/NewProject/Builds/iOS/Rhythm Pattern Explorer iPad/Images.xcassets/AppIcon.appiconset"
    
    print("🎵 Generating Rhythm Pattern Explorer Icons...")
    
    # Create base SVG
    base_svg = create_base_svg(1024)
    
    # Generate all sizes
    for filename, size in icon_sizes.items():
        print(f"   Creating {filename} ({size}x{size})")
        
        # Convert SVG to PNG at target size
        img = svg_to_png(base_svg, size)
        
        # Save to output directory
        output_path = os.path.join(output_dir, filename)
        img.save(output_path, 'PNG', optimize=True)
    
    print(f"✅ Generated {len(icon_sizes)} icon files")
    print(f"📁 Saved to: {output_dir}")
    print("\n🎨 Icon Design: Accented Quintillo E(5,8) with {10} accent")
    print("   • Pattern: 10110100 (quintillo rhythm)")
    print("   • Green dots: regular onsets")  
    print("   • Red dots: accented onsets (positions 0,2)")
    print("   • Gray dots: rests")

if __name__ == "__main__":
    try:
        generate_all_icons()
    except ImportError as e:
        print("❌ Missing dependencies. Install with:")
        print("   pip install cairosvg pillow")
    except Exception as e:
        print(f"❌ Error: {e}")