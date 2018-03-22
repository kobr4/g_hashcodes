main()

function main(){

	var startTime = new Date().getTime()

	var inputs = [
		"a_example.in",
		"b_should_be_easy.in",
		"c_no_hurry.in",
		"d_metropolis.in",
		"e_high_bonus.in"
	]

	var score = 0

	var i = 0
	while(i<inputs.length) {

		var dataset = parse(inputs[i])

		var data = {
			name: inputs[i],
			gridRows: dataset[0][0],
			gridCols: dataset[0][1],
			carCount: dataset[0][2],
			rideCount: dataset[0][3],
			bonus: dataset[0][4],
			stepLimit: dataset[0][5],
			rides: getRides(dataset)
		}

		computeStats(data)

		var cars = solve(data)

		score = score + computeResults(cars)

		printOutput(inputs[i].split(".")[0] + ".out", cars)

		i++
	}

	console.log("\n*** FINAL SCORE ***\n\n" + score)

	console.log("\nexec time: " + (new Date().getTime() - startTime) + " ms")
}



// ***********

function solve(data) {

	console.log("\n*** starting input: "+ data.name + "***\n")
	
	var rides = data.rides
	var bonus = data.bonus
	var stepLimit = data.stepLimit
	var carCount = data.carCount

	var cars = []
	
	var i = carCount

	while(i>0) {

		var car = {
			id:i,
			step: 0,
			position: [0,0],
			rides: [],
			length: 0,
			bonus: 0,
			wait: 0,
			travel: 0,
		}

		if(i%100 == 0)
			console.log((carCount - i) + " cars computed")

		while(car.step < stepLimit - 1) {

			if(rides.length === 0) {
				// car.step = stepLimit - 1
				break
			}

			var nextRide = null
			var nextStepsToStart = stepLimit - car.step
			var nextBonus = 0
			var nextWait = 0
			var nextTravel = 0

			rides.forEach(r => {
				var stepsToTravel = getDistance(car.position, r.startPosition)
				var stepsToWait = r.start - stepsToTravel <= car.step ? 0 : r.start - stepsToTravel - car.step
				var stepsToStart = stepsToTravel + stepsToWait
				var isMissed = car.step + stepsToStart > r.startLimit
				// console.log()
				if(!isMissed){
					var bonusValue = car.step + stepsToTravel <= r.start ? bonus : 0
					if(stepsToStart < nextStepsToStart){
						nextRide  = r
						nextTravel = stepsToTravel
						nextWait = stepsToWait
						nextStepsToStart = stepsToStart
						nextBonus = bonusValue
					}
				}
			})

			// console.log(JSON.stringify(nextRide))

			if(nextRide === null) {
				// car.step = stepLimit - 1
				break
			}

			car.rides.push(nextRide.id)
			car.step = car.step + nextStepsToStart + nextRide.length
			car.position = nextRide.finishPosition.slice()
			car.length = car.length + nextRide.length
			car.travel = car.travel + nextStepsToStart
			car.wait = car.wait + nextWait
			car.bonus = car.bonus + nextBonus
			rides.splice(rides.findIndex(r => r.id === nextRide.id),1)

		}

		// console.log("\n"+JSON.stringify(car)+"\n")

		cars.push(car)

		i--
	}

	return cars

}








// ******** fns

function parse(fname){

	var str = require("fs").readFileSync(fname).toString()
	// console.log(str)
	var data = []
	var rows = str.split("\n")
	var i = 0
	while(i<rows.length) {
		if(rows[i]) {
			var row = []
			// console.log(rows[i])
			var chunks = rows[i].split(" ")
			var j = 0
			while (j<chunks.length){
				// console.log(chunks[j])
				row.push(parseInt(chunks[j]))
				j++
			}
			data.push(row)
		}
		i++
	}
	// console.log(JSON.stringify(data))
	return data
}

function getDistance(pos1, pos2) {
	var x = pos1[0] - pos2[0] < 0? pos2[0] - pos1[0] : pos1[0] - pos2[0]
	var y = pos1[1] - pos2[1] < 0? pos2[1] - pos1[1] : pos1[1] - pos2[1]
	return x + y
}

function getRides(data) {

	var bonus = data[0][4]
	var rides = []

	var i = 1
	while(i<data.length){
		var ride = {
			id: i-1,
			startPosition: [data[i][0],data[i][1]],
			finishPosition: [data[i][2],data[i][3]],
			start: data[i][4],
			finish: data[i][5],
		}
		ride.length = getDistance(ride.startPosition,ride.finishPosition)
		ride.startLimit = ride.finish - ride.length
		var initialDistance = ride.startPosition[0] + ride.startPosition[1]
		var initialWait = ride.start < initialDistance ? 0 : ride.start - initialDistance
		rides.push(ride)
		i++
	}

	return rides
}

function printOutput(fname, cars){

	var output = ""

	i = cars.length - 1

	while(i>=0){
		var car = cars[i]
		var rides = car.rides
		var line = rides.length
		var j = 0

		while(j<rides.length) {
			line = line + " " + rides[j]
			j++
		}
		// console.log(line)
		output = output + line + "\n"
		i--
	}

	require("fs").writeFileSync(fname,output)
}

function computeStats(data) {

	console.log("\n*** Stats for " + data.name + " ***\n")
	console.log("ride count : " + data.rideCount)
	console.log("car count : " + data.carCount)
	console.log("step count : " + data.stepLimit)
	console.log("bonus value : " + data.bonus)

	var eligibleForBonus = 0

	var minLength = data.stepLimit
	var maxLength = 0
	var sumLength = 0

	var minStart = data.stepLimit
	var maxStart = 0
	var sumStart = 0

	var minFinish = data.stepLimit
	var maxFinish = 0
	var sumFinish = 0

	var minWindow = data.stepLimit
	var maxWindow = 0
	var sumWindow = 0

	data.rides.forEach(function(r){
		if(r.startPosition[0] + r.startPosition[1] <= r.start)
			eligibleForBonus ++
		if(minLength > r.length)
			minLength = r.length
		if(maxLength < r.length)
			maxLength = r.length
		sumLength = sumLength + r.length
		if(minStart > r.start)
			minStart = r.start
		if(maxStart < r.start)
			maxStart = r.start
		sumStart = sumStart + r.start
		if(minFinish > r.finish)
			minFinish = r.finish
		if(maxFinish < r.finish)
			maxFinish = r.finish
		sumFinish = sumFinish + r.finish
		var rWindow = r.finish - r.start
		if(minWindow > rWindow)
			minWindow = rWindow
		if(maxWindow < rWindow)
			maxWindow = rWindow
		sumWindow = sumWindow + rWindow
	})

	console.log("\nsum ride length : " + sumLength)
	console.log("min ride length : " + minLength)
	console.log("max ride length : " + maxLength)
	console.log("avg ride length : " + sumLength/data.rideCount)

	console.log("\nsum ride start : " + sumStart)
	console.log("min ride start : " + minStart)
	console.log("max ride start : " + maxStart)
	console.log("avg ride start : " + sumStart/data.rideCount)

	console.log("\nsum ride finish : " + sumFinish)
	console.log("min ride finish : " + minFinish)
	console.log("max ride finish : " + maxFinish)
	console.log("avg ride finish : " + sumFinish/data.rideCount)

	console.log("\nsum ride window : " + sumWindow)
	console.log("min ride window : " + minWindow)
	console.log("max ride window : " + maxWindow)
	console.log("avg ride window : " + sumWindow/data.rideCount)


	console.log("\nbonus eligible rides count : " + eligibleForBonus)
	console.log("not possible bonus rides count : " + (data.rideCount - eligibleForBonus))
	console.log("bonus potential : " + (data.bonus * eligibleForBonus))
	console.log("potential score : " + (sumLength + data.bonus * eligibleForBonus))

}

function computeResults(cars) {

	console.log("\n*** Results ***")

	var minLength = 1000000
	var maxLength = 0
	var sumLength = 0

	var minWait = 1000000
	var maxWait = 0
	var sumWait = 0

	var minTravel = 1000000
	var maxTravel = 0
	var sumTravel = 0

	var minBonus = 1000000
	var maxBonus = 0 
	var sumBonus = 0

	var minFinish = 1000000
	var maxFinish = 0
	var sumFinish = 0

	var minScore = 1000000
	var maxScore = 0
	var sumScore = 0

	cars.forEach(r => {
		if(r.length < minLength)
			minLength = r.length
		if(r.length > maxLength)
			maxLength = r.length
		sumLength = sumLength + r.length
		if(r.wait < minWait)
			minWait = r.wait
		if(r.wait > maxWait)
			maxWait = r.wait
		sumWait = sumWait + r.wait
		if(r.travel < minTravel)
			minTravel = r.travel
		if(r.travel > maxTravel)
			maxTravel = r.travel
		sumTravel =  sumTravel + r.travel
		if(r.bonus < minBonus)
			minBonus = r.bonus
		if(r.bonus > maxBonus)
			maxBonus = r.bonus
		sumBonus =  sumBonus + r.bonus
		var score = r.length + r.bonus
		if(score < minScore)
			minScore = score
		if(score > maxScore)
			maxScore = score
		sumScore =  sumScore + score
		if(r.step < minFinish)
			minFinish = r.step
		if(r.step > maxFinish)
			maxFinish = r.step
		sumFinish = sumFinish + r.step
	})

	console.log("\nsum length : " + sumLength)
	console.log("min length : " + minLength)
	console.log("max length : " + maxLength)
	console.log("avg length : " + sumLength/cars.length)

	console.log("\nsum wait : " + sumWait)
	console.log("min wait : " + minWait)
	console.log("max wait : " + maxWait)
	console.log("avg wait : " + sumWait/cars.length)

	console.log("\nsum travel : " + sumTravel)
	console.log("min travel : " + minTravel)
	console.log("max travel : " + maxTravel)
	console.log("avg travel : " + sumTravel/cars.length)

	console.log("\nsum bonus : " + sumBonus)
	console.log("max bonus : " + maxBonus)
	console.log("min bonus : " + minBonus)
	console.log("avg bonus : " + sumBonus/cars.length)

	console.log("\nmin finish : " + minFinish)
	console.log("max finish : " + maxFinish)
	console.log("avg finish : " + sumFinish/cars.length)

	console.log("\nsum score : " + sumScore)
	console.log("min score : " + minScore)
	console.log("max score : " + maxScore)
	console.log("avg score : " + sumScore/cars.length)

	return sumScore
}