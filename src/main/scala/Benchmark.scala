import org.apache.spark.SparkContext
import org.apache.spark.SparkContext._
import org.apache.spark.SparkConf
import org.apache.spark.scheduler._
import javax.xml.bind.DatatypeConverter
import scala.collection.mutable.MutableList

object Benchmark {
  def main(args: Array[String]) {
    val pipeTimes = Mutable()
    val conf = new SparkConf().setAppName("Microbenchmark")
    val sc = new SparkContext(conf)
    sc.addSparkListener(new SparkListener() {
        override def onTaskEnd(taskEnd: SparkListenerTaskEnd): Unit = {
          println("stageId %d, time %d".format(taskEnd.stageId, taskEnd.taskInfo.duration))
          if (taskEnd.stageId == 1) {
            pipeTimes += taskEnd.taskInfo.duration
          }
        }
      });

    val images_with_paths =
      sc.binaryFiles("gs://vdb-imagenet/kcam/frames/*")
    val images = images_with_paths.map(x => x._2).repartition(64)
    val b64images = images.map(x =>
      DatatypeConverter.printBase64Binary(x.toArray()))

    println("after images");

    val start = System.nanoTime()

    var i = 0
    var output = 0
    for ( i <- 1 to 1 ) {
      val b64features = b64images.pipe("./evaluate_caffe")
      val features = b64features.map(x =>
        DatatypeConverter.parseBase64Binary(x))
      val count = features.count()
      println("features count %d\n".format(count));
    }

    val end = System.nanoTime()
    val t = (end - start) / (1 * 1000000.0);

    println("RDD output (%f ms): %d".format(t, output))

    pipeTimes.foreach { println }
  }
}
