<?php
// Start this file from command prompt -> $php -f nextcloud_chat_bot.php
set_time_limit(0);

$SERVER = "https://mycloud.com";
$USER = "{automation}"; // First you will have to create this user in your Nextcloud instance or use existing one.
$PASS = "{automationpass}";
$channel_id = 'channelOrRoomId';
$occ  = "/mnt/archive/nextcloud/occ";
$php  = "/mnt/archive/httpd/php";

function request($uri,$post = false){
	global $USER, $PASS;
	$payload = '';
	$ch = curl_init($uri);
	if($post){
		$payload = json_encode($post);
		curl_setopt($ch, CURLOPT_POST, true);
		curl_setopt($ch, CURLOPT_POSTFIELDS, $payload);
	}
		curl_setopt($ch, CURLOPT_CONNECTTIMEOUT, 0); 
		curl_setopt($ch, CURLOPT_TIMEOUT, 400); //timeout in seconds
		curl_setopt($ch, CURLOPT_SSL_VERIFYPEER, true);
		curl_setopt($ch, CURLOPT_RETURNTRANSFER, true);
		curl_setopt($ch, CURLINFO_HEADER_OUT, true);
		curl_setopt($ch, CURLOPT_USERPWD, "$USER:$PASS");
		curl_setopt($ch, CURLOPT_HTTPAUTH, CURLAUTH_BASIC);
		curl_setopt($ch, CURLOPT_HTTPHEADER, array(
			'Content-Type: application/json',
			'Content-Length: ' . strlen($payload),
			'Accept: application/json',
			'OCS-APIRequest: true'));
	$result = json_decode(curl_exec($ch));
	curl_close($ch);
	return $result;
}

function NextcloudTalk_ReadLatest($lastKnownMessageId) {
	global $SERVER, $channel_id;
	$uri = "${SERVER}/ocs/v2.php/apps/spreed/api/v1/chat/${channel_id}?lookIntoFuture=1&timeout=5&lastKnownMessageId=$lastKnownMessageId&includeLastKnown=0";
   return request($uri);
}

function NextcloudTalk_SendMessage($message) {
	global $SERVER, $channel_id;
	$uri = "${SERVER}/ocs/v2.php/apps/spreed/api/v1/chat/${channel_id}";
	return request($uri,array("token" => $channel_id, "message" => $message));
}

$lastMsg = NextcloudTalk_SendMessage('Bot Starting.');
$lastId = $lastMsg->ocs->data->id;

while(true){
	$lastMsg = NextcloudTalk_ReadLatest($lastId);
	if($lastMsg)
	{
		$article = $lastMsg->ocs->data[0];
		$actor = $article->actorId;
		$lastId = $article->id;
		$isForBot = $USER === $article->messageParameters->{'mention-user1'}->id;
		$keyword = trim(str_replace('{mention-user1}','',strtolower($article->message)));
		
		if($isForBot){

			if($keyword === "load"){
				$lastMsg = NextcloudTalk_SendMessage("@$actor Current load is: " . @file_get_contents('/proc/loadavg') );
				$lastId = $lastMsg->ocs->data->id;
			}

			if($keyword === "uptime"){
				$seconds = current(explode(" ",@file_get_contents('/proc/uptime')));
				$hours = floor($seconds / 3600);
				$mins = floor($seconds / 60 % 60);
				$secs = floor($seconds % 60);
				$timeFormat = sprintf('%02dh %02dm %02ds', $hours, $mins, $secs);
				$lastMsg = NextcloudTalk_SendMessage("@$actor Uptime is: " . $timeFormat );
				$lastId = $lastMsg->ocs->data->id;
			}
					
			if($keyword === "sensors"){
				NextcloudTalk_SendMessage("@$actor");
				NextcloudTalk_SendMessage(`sensors`);
			}
			
			if($keyword === "weather"){
				NextcloudTalk_SendMessage("@$actor");
				NextcloudTalk_SendMessage(`curl -s wttr.in/{Jambol,Sofia,Moscow}?format="%c+%t+%l\\n" 2>&1`);
			}
			
			if($keyword === "users"){
				NextcloudTalk_SendMessage("@$actor");
				NextcloudTalk_SendMessage(`sudo -u www-data $php $occ user:list`);
			}
			
			if($keyword === "integrity"){
				NextcloudTalk_SendMessage("@$actor");
				NextcloudTalk_SendMessage(`sudo -u www-data $php $occ integrity:check-core`);
				NextcloudTalk_SendMessage("@$actor Integrity check Done");
			}

			if($keyword === "lock"){
				NextcloudTalk_SendMessage('@'.$actor.' '. `loginctl lock-session && xset -display :0.0 dpms force off`);
			}

			if($keyword === "unlock"){
				NextcloudTalk_SendMessage('@'.$actor.' '. `loginctl unlock-session && xset -display :0.0 dpms force on`);
			}

			if($keyword === "heater on"){
				NextcloudTalk_SendMessage('@'.$actor.' '. `curl -s "https://mycloud.com/index.php/apps/smartdev/api/1.0/setstate?id={heaterDeviceId)&state=1" -X GET -u 'admin:adminpass'`);
			}
			
			if($keyword === "heater off"){
				NextcloudTalk_SendMessage('@'.$actor.' '. `curl -s "https://mycloud.com/index.php/apps/smartdev/api/1.0/setstate?id={heaterDeviceId}&state=0" -X GET -u 'admin:adminpass'`);
			}

			if($keyword === "help"){
				NextcloudTalk_SendMessage(implode("\n",[
					"@$actor",
					"1) Load - Display system load.",
					"2) Uptime - Display system uptime.",
					"3) Sensors - Display system sensors.",
					"4) Weather - Display weather for preprogrammed locations.",
					"5) Users - Display Nextcloud users.",
					"6) Integrity - Run integrity check of the core.",
					"7) Lock - Lock workstation.",
					"8) Unlock - Unlock workstation."
				]));
			}
		}
	}
}
