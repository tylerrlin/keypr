import UIKit

// Get all font families
print("starting!")

for family in UIFont.familyNames.sorted() {
    print("Family: \(family)")
    
    // Get all font names for this family
    for name in UIFont.fontNames(forFamilyName: family) {
        print("    \(name)")
    }
}
