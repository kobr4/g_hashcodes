package hashcode2019

case class Photo(id: Int, tags: List[Long], o: Char, interest: Int = 0)

case class Slide(tags: List[Long], pList: List[Photo], interest: Int = 0) {
  override def toString: String = {
    s"${pList.map(_.id).mkString(",")}"
  }
}

object Slide extends {

  def apply(photo: Photo): Slide = {
    Slide(photo.tags, List(photo))
  }

  def apply(p1: Photo, p2: Photo): Slide = {
    Slide(p1.tags.union(p2.tags), List(p1, p2))
  }
}

object Solver extends App {

  val photoList = Helper.loadInputFile(Helper.filename)

  println(s"Nb photos: ${photoList.length}")

  val slides = Helper.buildSlideList(photoList)

  println(s"Nb slides: ${slides.length}")

/*
  val solution = Helper.sortSlide(slides.sortBy(_.tags.length)).grouped(5000).toList.flatMap { split =>
    Helper.greedy(Nil,split)
  }
*/

  val solution = Helper.greedy(Nil, slides)

  println("-- Writing output --")

  Helper.ouput(solution, Helper.filename)
}

