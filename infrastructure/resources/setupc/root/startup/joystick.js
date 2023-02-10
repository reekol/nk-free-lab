#!/usr/bin/env node

/*
 * Documentation:
 * https://www.kernel.org/doc/Documentation/input
 * https://www.kernel.org/doc/Documentation/input/joystick-api.txt
 *
 */

const fs = require("fs")
const path = require("path")
const http = require("http")

const DEV_INPUT = "/dev/input/js0"

const httphook = url => {
    const   req = http.request(url, res => {
            console.log(`statusCode: ${res.statusCode}`)
            res.on('data', d => process.stdout.write(d) )
    })
    req.on('error', error => console.error(error) )
    req.end()
}

let events_cache = {}

const ACTIONS = {
//      type:number
    '1:7': data => {
        if(events_cache['1:5'] && events_cache['1:5'].value){
            httphook('http://192.168.0.52/relay/0?turn=on' )
            setTimeout(() => httphook('http://192.168.0.52/relay/0?turn=off'), 400)
        }
    },
    '1:8': data => httphook('http://192.168.0.54/color/0?turn=off'),
    '1:0': data => {
        httphook('http://192.168.0.54/settings/?mode=white')
        if(events_cache['1:5'] && events_cache['1:5'].value){
            httphook('http://192.168.0.54/white/0?temp=3000&brightness=1')
        }else{
            httphook('http://192.168.0.54/white/0?temp=3000&brightness=100')
        }
         httphook('http://192.168.0.54/color/0?turn=on')
    },
    '1:1': data => {
        httphook('http://192.168.0.54/settings/?mode=color')
        if(events_cache['1:5'] && events_cache['1:5'].value){
            httphook('http://192.168.0.54/color/0?red=255&green=0&blue=0&white=0&gain=1')
        }else{
            httphook('http://192.168.0.54/color/0?red=255&green=0&blue=0&white=0&gain=100')
        }
         httphook('http://192.168.0.54/color/0?turn=on')
    },
    '1:2': data => {
        httphook('http://192.168.0.54/settings/?mode=color')
        if(events_cache['1:5'] && events_cache['1:5'].value){
            httphook('http://192.168.0.54/color/0?red=0&green=255&blue=0&white=0&gain=1')
        }else{
            httphook('http://192.168.0.54/color/0?red=0&green=255&blue=0&white=0&gain=100')
        }
         httphook('http://192.168.0.54/color/0?turn=on')
    },
    '1:3': data => {
        httphook('http://192.168.0.54/settings/?mode=color')
        if(events_cache['1:5'] && events_cache['1:5'].value){
            httphook('http://192.168.0.54/color/0?red=0&green=0&blue=255&white=0&gain=1')
        }else{
            httphook('http://192.168.0.54/color/0?red=0&green=0&blue=255&white=0&gain=100')
        }
         httphook('http://192.168.0.54/color/0?turn=on')
    }


}

const input = fs.createReadStream(DEV_INPUT)
      input.on('error', error => console.log)
      input.on('data', data => {
            let time   = data.readUInt32LE(0)
            let value  = data.readInt16LE(4)
            let type   = data.readUInt8(6)
            let number = data.readUInt8(7)
            let action = [type,number].join(':')
                events_cache[action] = {time: time, value: value }
           console.log(['LOG', time,value,type,number, action])
           if(typeof ACTIONS[action] !== 'undefined') ACTIONS[action]({time: time, value: value, type: type, number: number})
      })
