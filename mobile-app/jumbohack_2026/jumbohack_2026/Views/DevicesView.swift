//
//  DevicesView.swift
//  jumbohack_2026
//
//  Created by Elizabeth Ngai on 2/21/26.
//

//TODO: actually make the connection logic lolz

import SwiftUI

//TODO: stand-ins for
let dummy_keypr_list = ["keypr-1", "BLE Passkey", "Test Device"]
let dummy_mobile_devices_list = ["Jack's Computer"]

struct DevicesView: View {
    var keyprs: [String]
    var computers: [String]

    func hardware_keys(device: String) -> some View {
        HStack {
            Text(device)
                .foregroundColor(AppColors.whiteSubtext)
                .font(.custom("KodeMono-Regular", size: 18))
            Spacer()
            is_connected(device: device)
        }
    }
    
    func is_connected(device: String) -> some View {
        let connection_logic = device == "keypr-1" ? true : false
        if connection_logic {
            return Text("connected")
                .foregroundStyle(AppColors.blueAccent)
                .font(.custom("KodeMono-Regular", size: 18))
        } else {
            return Text("not connected")
                .foregroundStyle(Color.gray)
                .font(.custom("KodeMono-Regular", size: 18))
        }
            
    }
    var body: some View {
        VStack (alignment: .leading, spacing: 0){
           Text("Devices")
               .font(.custom("KodeMono-Regular", size: 32))
               .foregroundColor(.white)
//               .padding(.top, 20)
               .bold()
               .frame(maxWidth: .infinity, alignment: .center)
           Text("hardware keys")
               .font(.custom("KodeMono-Regular", size: 24))
               .foregroundStyle(AppColors.goldMainText)
               .padding(.top, 20)
           Divider()
               .frame(height: 2)
               .overlay(AppColors.whiteSubtext)
               .padding(.top, 4)
           
            
           // Show the device names in a list
           ScrollView {
               VStack(spacing: 12) {
                   ForEach(keyprs, id: \.self) { device in
                       hardware_keys(device: device)
                   }
               }
           }
           .padding(.top, 16)
           .frame(height: 150)
            
            Text("mobile devices")
                .font(.custom("KodeMono-Regular", size: 24))
                .foregroundStyle(AppColors.goldMainText)
            Divider()
                .frame(height: 2)
                .overlay(AppColors.whiteSubtext)
                .padding(.top, 4)
            
             
            // Show the device names in a list
            ScrollView {
                VStack(spacing: 12) {
                    ForEach(computers, id: \.self) { device in
                        hardware_keys(device: device)
                    }
                }
            }
            .padding(.top, 16)
            .frame(height: 150)
           Spacer()
       }
        .padding(.top, 16)
        .padding(.horizontal, 25)
       .background(AppColors.blackBackground)
       
   }
}


struct DevicesViewPreview: PreviewProvider {
    static var previews: some View {
        DevicesView(keyprs: dummy_keypr_list,
                    computers: dummy_mobile_devices_list)
            .previewDevice("iPhone 14 Pro") // optional: pick device
            .preferredColorScheme(.dark)    // optional: dark/light mode

    }
}
