/*
Author: Feng YANG
Desc: Google Hashcode 2015 qualification code
*/
　
var _startTime = new Date().getTime()
　
var parse = () => {
　
	var fs = require("fs")
　
	var data = fs.readFileSync("dc.in").toString()
　
	// console.log(data)
　
	var lines = data.split("\n")
　
	// console.log("lines count : "+lines.length)
　
	var i = 0
　
	var dataset = []
　
	while (i<lines.length) {
		var line = lines[i]
		var values = line.split(' ')
		
		var j = 0
		while(j<values.length) {
			values[j] = parseInt(values[j])
			j++
		}
　
		// console.log(typeof values[0])
　
		dataset.push(values)
　
		i++
	}
	return dataset
}
　
var dataset = parse()
　
var affect_server = () => {
　
	var servers = []
　
	var n_server = dataset[0][4]
	var n_unavailable = dataset[0][2]
　
	var i = 0
　
	while (i< dataset.length - 1 - n_unavailable - 1) {
　
		var server = {
			id: i,
			size: dataset[i+n_unavailable+1][0],
			capacity: dataset[i+n_unavailable+1][1],
			capacityByCell: dataset[i+n_unavailable+1][1]/dataset[i+n_unavailable+1][0],
			pool: null
		}
　
		servers.push(server)
　
		i++
	}
　
　
	servers = servers.sort((s1,s2) => s2.capacityByCell - s1.capacityByCell !== 0 ?
			s2.capacityByCell - s1.capacityByCell
			: s1.size - s2.size
		)
　
	// console.log(JSON.stringify(servers.map(s => s.capacityByCell)))
　
	return servers
　
}
　
var affect_rows = () => {
　
	var n_rows = dataset[0][0]
	var n_slots = dataset[0][1]
	var n_unavailable = dataset[0][2]
　
	var grid = []
　
	var r = 0
　
	while(r< n_rows) {
		var row = []
		var s = 0
		while(s<n_slots) {
			row.push(0)
			s++
		}
		grid.push(row)
		r++
	}
　
	var u = 0
	while (u< n_unavailable) {
　
		var x = dataset[u+1][0]
		var y = dataset[u+1][1]
		grid[x][y] = null
		u++
	}
　
	// console.log(JSON.stringify(grid))
　
	var rows = []
　
	r=0
　
	while(r< n_rows) {
		
		row = {row_id:r, ranges: [], n_ranges:1, n_unavailable: 0,freeSlots: 0, capacity: 0, servers: []}
　
		s = 0
　
		var range_id = 0
　
		var start = null
		var size = 0
　
		while (s<n_slots) {
　
			if (start === null) {
				if(grid[r][s] !== null) {
					start = s
					size = 1
					row.freeSlots++
　
					if(s === n_slots-1) {
						row.ranges.push({
							range_id,
							start,
							size,
							left: size
						})
						start = null
						size = 0
						row.n_ranges++
					}
				}
				else {
					row.n_unavailable++
				}
　
			}
			else {
				if(grid[r][s] === null) {
					row.ranges.push({
						range_id,
						start,
						size,
						left: size
					})
					start = null
					size = 0
					row.n_ranges++
					range_id++
					row.n_unavailable++
				}
				else {
					size++
					row.freeSlots++
					if(s === n_slots-1) {
						row.ranges.push({
							range_id,
							start,
							size,
							left: size
						})
						row.n_ranges++
					}
				}
			}
			s++
		}
		
		var row_size = row.n_unavailable
		var freeSlots = 0
		// console.log(r +" null : "+row.n_unavailable)
		row.ranges.forEach(rec => row_size = row_size + rec.size)
		row.ranges.forEach(rec => freeSlots = freeSlots + rec.size)
		// console.log(r +" range count : "+ (range_id + 1))
		// console.log(r +" size : "+row_size)
		// console.log(r +" freeSlots : "+freeSlots)
　
		rows.push(row)
　
		r++
　
	}
　
	// console.log(JSON.stringify(rows))
　
	return rows
　
}
　
servers = affect_server()
　
rows = affect_rows()
// var total_capacity = 0
// var total_available_slots = 0
// var total_servers_size = 0
　
// servers.forEach(r => {
// 	total_capacity = total_capacity + r.capacity
// 	total_servers_size = total_servers_size + r.size
// })
　
// rows.forEach(row => {
// 	var available_slots = 0
// 	row.ranges.forEach(range => available_slots = available_slots + range.size)
// 	total_available_slots = total_available_slots + available_slots
// })
　
// console.log("total_capacity : " + total_capacity)
// console.log("total_servers_size : " + total_servers_size)
// console.log("total_available_slots : " + total_available_slots)
　
　
　
var availableServers = JSON.parse(JSON.stringify(servers))
var usedServers = []
var filledRows = JSON.parse(JSON.stringify(rows))
　
var placeNextBestRatioServer = (rowId) => {
	var row_processing = filledRows[rowId]
	// var largestRangeWidth = 0
	var smallestRangeWidth = 1000
	var positionY = null
	var rangeId = null
　
	// compute smallest range left
	var i = 0
	while(i<row_processing.ranges.length) {
		var range = row_processing.ranges[i]
		if(range.left > 0 && range.left <= smallestRangeWidth) {
			smallestRangeWidth = range.left
			positionY = range.start + range.size - range.left
			rangeId = range.range_id
		}
		i++
	}
　
	// console.log(JSON.stringify({
	// 	row_processing,
	// 	smallestRangeWidth,
	// 	positionY,
	// 	rangeId
	// }))
　
	if(smallestRangeWidth < 100) {
		// find the best ratio server to fit in that range
		i=0
		while(i<availableServers.length) {
			// found the best ratio server to fit
	 		if(availableServers[i].size <= smallestRangeWidth) {
	 			var server = availableServers[i]
	 			// console.log("found server to fit : "+JSON.stringify(server))
	 			// copy server to used list
	 			usedServers.push(Object.assign(
	 				{
	 					position: [rowId,positionY]
	 				},
	 				server
	 			))
	 			row_processing.servers.push(Object.assign(
	 				{
	 					position: [rowId,positionY]
	 				},
	 				server
	 			))
	 			// updating free slots
	 			row_processing.ranges[rangeId].left = row_processing.ranges[rangeId].left - server.size
	 			row_processing.freeSlots = row_processing.freeSlots - server.size
	 			row_processing.capacity = row_processing.capacity + server.capacity
	 			// remove server from available list
	 			// console.log("removing server : " + JSON.stringify(availableServers[i]) + " at index " + i)
	 			availableServers.splice(i,1)
	 			// console.log("availableServers : " + availableServers.length)
	 			break
	 		}
	 		i++
		}
	}
　
	
}
　
i = 0
　
while (1) {
　
	// compute number of free slots
	var freeSlots = 0
	k=0
	while (k<filledRows.length) {
		freeSlots = freeSlots + filledRows[k].freeSlots
		k++
	}
	if(freeSlots === 0) {
		console.log("All slots filled after " + i + " iterations")
		break
	}
	if(i>100000) {
		console.log("Giving up after 1.10^6 attempts")
		break
	}
　
	// console.log("freeSlots : " + freeSlots)
　
	var row_processing = i % 16
　
	// console.log("row_processing : " + row_processing)
　
	placeNextBestRatioServer(row_processing)
　
　
　
	i++
　
}
　
// console.log(JSON.stringify(usedServers))
// console.log("Number of server used : "+usedServers.length)
// console.log("Number of server unused : " +availableServers.length)
　
var unusedSlots = 0
filledRows.forEach(r => unusedSlots = unusedSlots + r.freeSlots)
　
// console.log("Number of unused slots : " + unusedSlots)
　
// filledRows.forEach(r => console.log("row " +r.row_id + " => total capacity : " + r.capacity))
　
var pools = []
　
var affectPools = () => {
	var n_pools = dataset[0][3]
　
	i = 0
　
	while (i<n_pools) {
		pools.push({
			id:i,
			servers: [],
			capacity: 0,
			rowCapacity: [],
			gc: 0,
			maxRowCapacity: 0
		})
		j=0
		while(j<filledRows.length) {
			pools[i].rowCapacity.push(0)
			j++
		}
		i++
	}
}
　
affectPools()
　
usedServers.sort((s1,s2) => s2.capacity - s1.capacity)
　
// console.log(JSON.stringify(usedServers.map(s => (s.capacity))))
　
var findBestPoolForMinGcIncrease = (server) => {
　
	var sortedPools = JSON.parse(JSON.stringify(pools.sort((p1,p2) => p1.gc - p2.gc)))
　
	var i = 0
	while(i<sortedPools.length) {
		
		var targetPool = sortedPools[i]
　
		targetPool.nextCapacity = targetPool.capacity + server.capacity
		targetPool.nextRowCapacity = targetPool.rowCapacity.slice()
		targetPool.nextMaxRowCapacity = targetPool.maxRowCapacity
		targetPool.nextGc = targetPool.gc
		targetPool.gcIncrease = 0
		targetPool.maxRowCapacityIncrease = 0
		targetPool.nextRowCapacity[server.position[0]] = targetPool.nextRowCapacity[server.position[0]] + server.capacity
		
		var j=0
　
		while(j<targetPool.nextRowCapacity.length) {
			if(targetPool.nextRowCapacity[j] > targetPool.nextMaxRowCapacity) {
				targetPool.nextMaxRowCapacity = targetPool.nextRowCapacity[j]
				targetPool.nextGc = targetPool.nextCapacity - targetPool.nextMaxRowCapacity
				targetPool.gcIncrease = targetPool.nextGc - targetPool.gc
				targetPool.maxRowCapacityIncrease = targetPool.nextMaxRowCapacity - targetPool.maxRowCapacity
			}
			j++
		}
　
		i++
	}
　
	// console.log(JSON.stringify(sortedPools))
　
	var sortedPools = sortedPools.sort((p1,p2) => p1.nextGc - p2.nextGc !== 0 ? 
		p1.nextGc - p2.nextGc 
		: p1.maxRowCapacityIncrease - p2.maxRowCapacityIncrease !== 0 ?
			p1.maxRowCapacityIncrease - p2.maxRowCapacityIncrease
			: p2.gcIncrease - p1.gcIncrease
	)
　
	// console.log(JSON.stringify(sortedPools))
　
	return sortedPools[0].id
}
　
　
i = 0
// while (i<50) {
while (i<usedServers.length) {
	var server = usedServers[i]
　
	// console.log(server.id)
	
	var targetPoolId = findBestPoolForMinGcIncrease(server)
　
	// console.log("targetPoolId : " + targetPoolId)
　
	var targetPool = null 
　
	var j = 0
	while(j<pools.length) {
		if(pools[j].id === targetPoolId) {
			targetPool = pools[j]
		}
		j++
	}
　
	server.pool = targetPoolId
　
	targetPool.servers.push(server)
　
	targetPool.capacity = targetPool.capacity + server.capacity
　
	targetPool.rowCapacity[server.position[0]] = targetPool.rowCapacity[server.position[0]] + server.capacity
　
	j=0
　
	while(j<targetPool.rowCapacity.length) {
		if(targetPool.rowCapacity[j] >= targetPool.maxRowCapacity) {
			targetPool.maxRowCapacity = targetPool.rowCapacity[j]
			targetPool.gc = targetPool.capacity - targetPool.maxRowCapacity
		}
		j++
	}
　
	i++
}
　
var minGc = 100000
var maxGc = 0
var totalServersCapacity = 0
var totalPoolCapacity = 0
var totalUsedServerCapacity = 0
var totalUnusedServerCapacity = 0
var minPoolCapacity = 100000
var maxPoolCapacity = 0
var minServerCount = 100000
var maxServerCount = 0
var serverCount = 0
　
pools.forEach(r => {
	// console.log("pool "+r.id + " : " + JSON.stringify(r))
	if(r.gc < minGc) minGc = r.gc
	if(r.gc > maxGc) maxGc = r.gc
	if(r.capacity < minPoolCapacity) minPoolCapacity = r.capacity
	if(r.capacity > maxPoolCapacity) maxPoolCapacity = r.capacity
	if(r.servers.length < minServerCount) minServerCount = r.servers.length
	if(r.servers.length > maxServerCount) maxServerCount = r.servers.length
	serverCount = serverCount + r.servers.length
	totalPoolCapacity = totalPoolCapacity + r.capacity
})
　
finalPools = pools.sort((p1, p2) => p1.id - p2.id)
finalServers = availableServers.concat(usedServers).sort((s1,s2) => s1.id-s2.id)
　
var unallocatedServer = 0
var allocatedServer = 0
　
var out = ""
　
finalServers.forEach(s => {
	var line = "\n" 
	totalServersCapacity = totalServersCapacity + s.capacity
	if(s.pool !== null) {
		line =s.position[0] + " " + s.position[1] + " " + s.pool+"\n"
		allocatedServer++
		totalUsedServerCapacity = totalUsedServerCapacity + s.capacity
	}
	else {
		line = "x\n"
		unallocatedServer++
		totalUnusedServerCapacity = totalUnusedServerCapacity + s.capacity
	}
	out = out + line
})
　
var fs = require("fs")
fs.writeFileSync("submission.out",out)
　
var execTime = new Date().getTime() - _startTime
　
console.log("\n*** Resuts stats ***\n")
console.log("total servers capacity :"+ totalServersCapacity)
console.log("total used servers capacity : "+totalUsedServerCapacity)
console.log("total unused servers capacity : "+totalUnusedServerCapacity)
console.log("total used capacity (assessed by pools) :"+totalPoolCapacity)
console.log("min pool capacity : "+minPoolCapacity)
console.log("max pool capacity : "+maxPoolCapacity)
console.log("max pool capacity diff : "+(maxPoolCapacity - minPoolCapacity))
console.log("total server count in pools : "+serverCount)
console.log("all servers allocated : " + (usedServers.length === serverCount))
console.log("total allocated servers : "+ allocatedServer)
console.log("total unallocated servers : "+unallocatedServer)
console.log("min server count by pool : "+minServerCount)
console.log("max server count by pool : "+maxServerCount)
console.log("min gc : "+minGc)
console.log("max gc : "+maxGc)
　
console.log("gc interval : "+(maxGc - minGc))
　
console.log("computed in : "+execTime+" ms")
　
console.log("\n*** FINAL SCORE ***\n" + minGc)
