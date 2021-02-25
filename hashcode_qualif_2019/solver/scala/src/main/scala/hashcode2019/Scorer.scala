package hashcode2019

object 3968Scorer extends App {

  val photolist = Helper.loadInputFile(Helper.filename)

  Helper.loadInputFile(Helper.filename)

  val slides = Helper.loadOuputFile(Helper.getOuputfilename(Helper.filename), photolist)

  println(s"Score: ${Helper.score(slides)}")

}
