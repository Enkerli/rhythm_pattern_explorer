# Target Selection Fix - CRITICAL

## Issue: Installing Extension Instead of App ❌

The error shows you're trying to install:
```
Rhythm%20Pattern%20Explorer%20iPad.appex  ← Extension file (wrong)
```

But iOS needs to install:
```
Rhythm Pattern Explorer iPad.app  ← Main app file (correct)
```

## Solution: Select Correct Target

### **In Xcode:**

1. **Look at the scheme selector** (top toolbar)
2. **You should see something like**:
   ```
   [Rhythm Pattern Explorer iPad] [Your iPad]
   ```
   
3. **Click the scheme dropdown** (left side)
4. **You should see options like**:
   ```
   • Rhythm Pattern Explorer iPad    ← SELECT THIS ONE
   • AUv3                           ← Not this one
   ```

5. **Select the main app target** (without "AUv3" in the name)

### **Visual Guide:**
```
Xcode Toolbar:
[▶] [Rhythm Pattern Explorer iPad ▼] [Alex's iPad ▼] [...]
     ↑ Click here to change scheme
```

### **Target Types:**
- **Main App Target**: `Rhythm Pattern Explorer iPad` 
  - ✅ This installs the container app + extension
  - ✅ This is what you run for testing
  
- **AUv3 Extension Target**: `AUv3`
  - ❌ This is just the extension part
  - ❌ Cannot be installed directly

## After Selecting Correct Target:

1. **Product → Clean Build Folder** (⌘+Shift+K)
2. **Product → Build** (⌘+B)
3. **Product → Run** (⌘+R)

## Expected Results:
- ✅ Installs main app: `Rhythm Pattern Explorer iPad.app`
- ✅ App contains AUv3 extension inside it
- ✅ App launches on iPad
- ✅ Extension gets registered with iOS

**Check your scheme selector now and make sure you have the main app target selected!** 🎯