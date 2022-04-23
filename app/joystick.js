"use strict"

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
         if(events_cache['1:5']){
            httphook('http://example.com/relay/0?turn=on' )
            setTimeout(() => httphook('http://example.com/relay/0?turn=off'), 400)
         }
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
                events_cache[action] = value
            console.log(['LOG', time,value,type,number, action])
            if(typeof ACTIONS[action] !== 'undefined') ACTIONS[action]({time: time, value: value, type: type, number: number})
      })
