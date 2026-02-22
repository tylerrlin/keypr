//
//  HomeView.swift
//  jumbohack_2026
//
//  Created by John Cha on 2/21/26.
//

// *should* contain the "connection status" of the keypr device

import SwiftUI

struct HomeView: View {
    
    var body: some View {
        VStack(alignment: .leading, spacing: 0) {
            Text("current status")
                .foregroundColor(AppColors.goldMainText)
                .font(.custom("KodeMono-Regular", size: 24))
                .padding(.bottom, 0)
            Divider()
                .frame(height: 2)
                .overlay(AppColors.whiteSubtext)
                .padding(.top, 2)
            HStack() {
                Text("keypr")
                    .foregroundColor(AppColors.whiteSubtext)
                    .font(.custom("KodeMono-Regular", size: 18))
                Spacer()
                Text("not connected")
                    .foregroundColor(.gray)
                    .font(.custom("KodeMono-Regular", size: 18))
            }.padding(.top, 25)
            HStack() {
                Text("authentication")
                    .foregroundColor(AppColors.whiteSubtext)
                    .font(.custom("KodeMono-Regular", size: 18))
                Spacer()
                Text("no requests")
                    .foregroundColor(.gray)
                    .font(.custom("KodeMono-Regular", size: 18))
            }.padding(.top, 10)
            
            Spacer()  // ← Spacer is now INSIDE the VStack, pushing empty space below
        }
        .padding(.top, 50)
        .padding(.horizontal, 25)
    }
}



struct HomeViewPreview: PreviewProvider {
    static var previews: some View {
        HomeView()
            .previewDevice("iPhone 14 Pro") // optional: pick device
            .preferredColorScheme(.dark)    // optional: dark/light mode
//        ContentView.printFonts()
            .background(AppColors.blackBackground)
    }
}
