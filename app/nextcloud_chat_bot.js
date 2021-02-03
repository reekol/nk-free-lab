"use strict"

const https = require('https')
const cp = require('child_process')
const d = console.log

const SERVER = "https://mycloud.com"
const USER = "automation" // User You should create for your bot.
const PASS = "automationPass"
const room = 'soMeId' // Chat RoomID
const ncroot = '/mnt/archive/nextcloud'
const occ = `${ncroot}/occ`
const php = "/mnt/archive//httpd/php"

let request = (uri, data = false) => {
	if(data) data = JSON.stringify(data)

	let options = { 
		method: data ? 'POST' : 'GET',
		headers: {
			'Authorization': 'Basic ' + Buffer.from(USER + ":" + PASS).toString('base64'),
			'Content-Type': 'application/json',
			'Accept':'application/json',
			'OCS-APIRequest': 'true'
		}
	}

	return new Promise((resolve, reject) => {
		const req = https.request(uri, options, (res) => {
			let responseBody = '';
			res.setEncoding('utf8');
			res.on('data', (chunk) => { responseBody += chunk })
			res.on('end', () => { 
				try{
					responseBody = JSON.parse(responseBody)
				}catch(e){
					d(responseBody)
					responseBody = false
				}
				resolve(responseBody)
			})
		})
		req.on('error', (err) => { reject(err) })
		if(data) req.write(data)
		req.end()
	})
}

let execSync = (cmd) => {
	return cp.execSync(cmd,{encoding:'utf8'})
}

let exec = async (cmd,cb) => {
	return cp.exec(cmd, async (err,stdout,stderr) => cb(stdout,stderr))
}

let sleep = (sec) => {
	return execSync(`sleep ${sec}`)
}

let NextcloudTalk_ReadLatest = async (lastKnownMessageId) => {
	let uri = `${SERVER}/ocs/v2.php/apps/spreed/api/v1/chat/${room}?lookIntoFuture=1&timeout=10&lastKnownMessageId=${lastKnownMessageId}&includeLastKnown=0`
   return await request(uri)
}

let NextcloudTalk_SendMessage = async (message) => {
	let uri = `${SERVER}/ocs/v2.php/apps/spreed/api/v1/chat/${room}`
	let r = await request(uri,{"token": room, "message": message})
	return r.ocs.data.id
}

let main = async () => {
	let lastId = await NextcloudTalk_SendMessage(`@${USER} Bot Starting.`)
	while(true){
		exec('sudo -u www-data date',o => d(o.trim())) // keep www-data active
		let lastMsg = await NextcloudTalk_ReadLatest(lastId)
		lastId = lastMsg ? lastMsg.ocs.data[0].id : lastId
		if(lastMsg) await	bot(lastMsg)
	}
}

let fastCmd = async (actor, cmd, cb = () => {}) => {
	exec(cmd, async (stdout,stderr) => {
		await NextcloudTalk_SendMessage(`@${actor}\n` + stdout)
		await cb(stdout,stderr)
	})
}

let bot = async (lastMsg) => {
	let article = lastMsg.ocs.data[0]
	let actor = article.actorId
	let lastId = article.id
	if(
		typeof article.messageParameters                      !== 'undefined' &&
		typeof article.messageParameters['mention-user1']     !== 'undefined' && 
		typeof article.messageParameters['mention-user1'].id  !== 'undefined' &&
		       article.messageParameters['mention-user1'].id  === USER
	) // Is adressed to the bot
	{
		let command = article.message.replace('{mention-user1}','').trim().split(' ')
		switch(command[0].toLowerCase()) {
		case "load":
			fastCmd(actor,`uptime | grep -ohe 'load average[s:][: ].*'`)
			break;
		case "uptime":
			fastCmd(actor,`uptime -p`)
			break;
		case "sensors":
			fastCmd(actor,`sensors`)
			break;
		case "weather":
			fastCmd(actor,`curl -s wttr.in/{Jambol,Sofia,Moscow}?format="%c+%t+%l\\n" 2>&1`)
			break;
		case "users":
			fastCmd(actor,`sudo -u www-data ${php} ${occ} user:list`)
			break;
		case "integrity":
			fastCmd(actor,`sudo -u www-data ${php} ${occ} integrity:check-core`)
			break;
		case "lock":
			fastCmd(actor,`loginctl lock-session && xset -display :0.0 dpms force off`)
			break;
		case "unlock":
			fastCmd(actor,`loginctl unlock-session && xset -display :0.0 dpms force on`)
			break;
		case "heater":
			let state = command[1] === 'on' ? 1 : 0;
			fastCmd(actor,`curl -s "${SERVER}/index.php/apps/smartdev/api/1.0/setstate?id={SomeDeviceId}&state=${state}" -X GET -u '${USER}:${PASS}'`)
			break;
		case "cam":
			let camFile = 'Cam-' + Date.now() + '.jpg';
			fastCmd(actor,`sudo -u www-data ffmpeg -i rtsp://192.168.1.12:8001/mpeg4 -f image2  -frames:v 1 -y ${ncroot}/data/${USER}/files/Talk/${camFile}`, async (stdout,stderr) => {
				execSync(`curl -s '${SERVER}/ocs/v2.php/apps/files_sharing/api/v1/shares' -H 'Content-Type: application/json' -H "OCS-APIRequest: true" -X POST -u '${USER}:${PASS}' --data-raw '{"path":"/Talk/${camFile}","permissions":19,"shareType":10,"shareWith":"${room}"}'`)
			})
			break;
		case "dvr":
			let sec = parseInt( command[1] ? command[1] : 10);
				 sec = sec < 60 * 60 * 6 ? sec : 60 * 60 * 6;
			let dvrFile = 'Cam-' + Date.now() + '.mp4';
			fastCmd(actor,`sudo -u www-data ffmpeg -i rtsp://192.168.1.12:8001/mpeg4 -vcodec copy -r 60 -t ${sec} -y ${ncroot}/data/${USER}/files/Talk/${dvrFile}`,async (stdout,stderr) => {
				execSync(`curl -s '${SERVER}/ocs/v2.php/apps/files_sharing/api/v1/shares' -H 'Content-Type: application/json' -H "OCS-APIRequest: true" -X POST -u '${USER}:${PASS}' --data-raw '{"path":"/Talk/${dvrFile}","permissions":19,"shareType":10,"shareWith":"${room}"}'`)
			})
			break;
		default:
			NextcloudTalk_SendMessage("@" + actor +  "\n" + [
					"1 ) Load - Display system load.",
					"2 ) Uptime - Display system uptime.",
					"3 ) Sensors - Display system sensors.",
					"4 ) Weather - Display weather for preprogrammed locations.",
					"5 ) Users - Display Nextcloud users.",
					"6 ) Integrity - Run integrity check of the core.",
					"7 ) Lock - Lock workstation.",
					"8 ) Unlock - Unlock workstation.",
					"9 ) Heater on - turns ON  smart device named heater (raquires smartdev app).",
					"10) Heater on - turns OFF smart device named heater (raquires smartdev app).",
					"11) Cam - creates image from rtsp camera and sends it to this chat.",
					"12) DVR [int] sec- Record video (duration in sec.) from rtsp."
					].join("\n"))
		}
	}
}
main()
