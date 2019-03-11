package hashcode2019

import java.io.{File, PrintWriter}

import scala.annotation.tailrec
import scala.io.Source

object Helper {

  val filename = "qualification_round_2019.in/a_example.txt"
  //val filename = "qualification_round_2019.in/b_lovely_landscapes.txt"
  //val filename = "qualification_round_2019.in/c_memorable_moments.txt"
  //val filename = "qualification_round_2019.in/d_pet_pictures.txt"
  //val filename = "qualification_round_2019.in/e_shiny_selfies.txt"

  def hash(in: String) : Long = in.toList.zipWithIndex.foldLeft(0L) { (acc, inc) => acc * 128 + inc._1 }

  def loadInputFile(filePath: String): List[Photo] = {
    val buffer = Source.fromFile(filePath).getLines
    buffer.take(1)

    (for {
      line <- buffer.zipWithIndex
    } yield {
      line._1.split(" ").toList match {
        case o :: n :: tail => Some(Photo(line._2, tail.map(hash(_)), o.toString.charAt(0)))
        case _ => None
      }
    }).flatten.toList
  }

  def loadOuputFile(filePath: String, photoList: List[Photo]) : List[Slide] = {
    Source.fromFile(filePath).getLines.drop(1).flatMap { line =>
      line.split(" ").map(_.toInt).toList match {
        case s1::s2::Nil => Option(Slide(photoList(s1), photoList(s2)))
        case s::Nil => Option(Slide(photoList(s)))
        case _ => None
      }
    }
  }.toList

  def score(slides: List[Slide]): Int = {
    slides.tail.foldLeft( (slides.head, 0) ){ (sAcc, slide) =>
      val intersect = sAcc._1.tags.intersect(slide.tags).length
      val m1 = sAcc._1.tags.length - intersect
      val m2 = slide.tags.length - intersect
      (slide, sAcc._2 + List(intersect,m1, m2).min)
    }._2
  }

  def buildSlideList(photoList: List[Photo]): List[Slide] = {
    //val a = photoList.filter(_.o == 'V').sortBy(_.tags.length).grouped(2).toList.map(p => Slide(p.flatMap(_.tags).distinct, p))
    val a = sortByInterest(Nil, photoList.filter(_.o == 'V')).grouped(2).toList.map(p => Slide(p.flatMap(_.tags).distinct, p))
    val b = photoList.filter(_.o == 'H').map(p => Slide(p.tags, List(p)))
    a ++ b
  }

  def sortSlide(slides: List[Slide]): List[Slide] = {
    val map = scala.collection.mutable.Map[Long, List[Slide]]()

    slides.foreach(s =>
      s.tags.foreach(t => map.put(t, map.getOrElse(t, List()) ::: List(s)))
    )

    map.toList.sortBy(_._2.length).reverse.flatMap(_._2).distinct
  }

  @tailrec
  def sortByInterest(head: List[Photo], slides: List[Photo]): List[Photo] = {
    //print(s"${head.length} - ${slides.length} - ")
    slides match {
      case Nil => head
      //case h :: tail if tail.length == 1 | tail.isEmpty => head ::: List(h) ::: tail
      case h :: tail =>

        val best = Photo(tail.head.id, tail.head.tags, tail.head.o, tail.head.tags.intersect(h.tags).length)
        val hSlide = tail.fold( best )(
          (s1: Photo, s2: Photo) => {
            if (s1.interest == 0)
              s1
            else {
              val hs1 = s1.interest
              val hs2 = h.tags.intersect(s2.tags).length
              if (hs1 < hs2)
                s1 else Photo(s2.id, s2.tags, s2.o, hs2)
            }
          })

        println(tail.length-1)
        sortByInterest(head :+ h :+ hSlide, tail.filter(_.id != hSlide.id))
    }
  }


  @tailrec
  def greedy(head: List[Slide], slides: List[Slide]): List[Slide] = {
    slides match {
      case Nil => head
      case h :: tail if tail.length == 1 => head:::List(h):::tail
      case h :: tail =>


        val hSlide = tail.par.map(s => Slide(s.tags, s.pList, s.tags.intersect(h.tags).length)).reduce(
          (s1: Slide, s2: Slide) => {
            //val hs1 = h.tags.intersect(s1.tags).length
            //val hs2 = h.tags.intersect(s2.tags).length
            val score1 = List(s1.tags.length - s1.interest, h.tags.length - s1.interest, s1.interest).min
            val score2 = List(s2.tags.length - s2.interest, h.tags.length - s2.interest, s2.interest).min
            if (score1 == score2)
              if (s1.tags.length < s2.tags.length) s1 else s2
            else if (List(s1.tags.length - s1.interest, h.tags.length - s1.interest, s1.interest).min  >
              List(s2.tags.length - s2.interest, h.tags.length - s2.interest, s2.interest).min)
              s1 else s2

          })

        println(tail.length)
        greedy(head :+ h, hSlide::tail.filter(_.pList != hSlide.pList))
      //greedy(head :+ h :+ hSlide, tail.filter(_.pList != hSlide.pList))
    }
  }

  def ouput(slides: List[Slide], filename: String) = {
    println(s"Nb slides in solution: ${slides.length}")

    val writer = new PrintWriter(new File(getOuputfilename(filename)))

    writer.write(slides.length.toString + "\n")

    slides.foreach(v => writer.write(v.pList.map(_.id - 1).mkString(" ") + "\n"))
    writer.close()
  }

  def getOuputfilename(fileInName: String): String = {
    fileInName.substring(fileInName.lastIndexOf('/') + 1).concat(".out")
  }
}