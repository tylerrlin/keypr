
//
//  AuthNotif.swift
//  jumbohack_2026
//
//  Created by John Cha on 2/21/26.
//

//This is the view for when a notification is hit

import SwiftUI

struct AuthNotifView: View {
    let appName: String
    let onAccept: () -> Void
    let onDecline: () -> Void
    let isPresent: Bool
    var body: some View {
        if (isPresent) {
            GeometryReader { geo in
                ZStack {
                    AppColors.blackBackground
                        .ignoresSafeArea()
                    
                    VStack {
                        Spacer(minLength: geo.size.height * 0.06)
                        
                        VStack(spacing: 0) {
                            Spacer(minLength: geo.size.height * 0.04)
                            
                            // Big warning/spy icon
                            Image("icon_authRequest")
                                .resizable()
                                .scaledToFit()
                                .frame(width: geo.size.width * 0.65)
                                .padding(.bottom, geo.size.height * 0.03)
                            
                            // AUTH REQUEST!
                            Text("AUTH REQUEST!")
                                .font(.custom("KodeMono-Regular_Bold", size: 35))
                                .foregroundStyle(AppColors.blueAccent)
                                .padding(.bottom, geo.size.height * 0.02)
                            
                            // keypr
                            Text(appName)
                                .font(.custom("KodeMono-Regular_SemiBold", size: 28))
                                .foregroundStyle(Color.black.opacity(0.85))
                                .padding(.bottom, 6)
                            
                            // is requesting access:
                            Text("is requesting access:")
                                .font(.custom("KodeMono-Regular", size: 22))
                                .foregroundStyle(Color.black.opacity(0.85))
                                .padding(.bottom, geo.size.height * 0.06)
                            
                            // Buttons row
                            HStack(alignment: .top, spacing: geo.size.width * 0.22) {
                                
                                // Decline
                                VStack(spacing: 10) {
                                    Button(action: onDecline) {
                                        Image("icon_lockClosed_grey")
                                            .resizable()
                                            .scaledToFit()
                                            .frame(width: geo.size.width * 0.18)
                                    }
                                    .buttonStyle(.plain)
                                    
                                    Text("decline")
                                        .font(.custom("KodeMono-Regular_Medium", size: 22))
                                        .foregroundStyle(Color.black.opacity(0.85))
                                }
                                
                                // Accept
                                VStack(spacing: 10) {
                                    Button(action: onAccept) {
                                        Image("icon_lockOpen")
                                            .resizable()
                                            .scaledToFit()
                                            .frame(width: geo.size.width * 0.18)
                                    }
                                    .buttonStyle(.plain)
                                    
                                    Text("accept")
                                        .font(.custom("KodeMono-Regular_Medium", size: 22))
                                        .foregroundStyle(AppColors.blueAccent)
                                }
                            }
                            
                            Spacer(minLength: geo.size.height * 0.06)
                        }
                        .frame(width: geo.size.width * 0.86)
                        .background(AppColors.goldMainText)
                        .clipShape(RoundedRectangle(cornerRadius: 6))
                        
                        Spacer(minLength: geo.size.height * 0.06)
                    }
                }
            }
        } else {
            EmptyView()
        }
    }
        
}
