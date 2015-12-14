import org.apache.spark.SparkContext
import org.apache.spark.SparkContext._
import org.apache.spark.SparkConf

object Benchmark {
  def main(args: Array[String]) {
    // Should be some file on your system
    val conf = new SparkConf().setAppName("Microbenchmark")
    val sc = new SparkContext(conf)

    val start = System.nanoTime()

    var i = 0
    var output = 0
    for ( i <- 1 to 20 ) {
      println("%d".format(i));
      val input = List.range(1, 1000000)
      val inputRDD = sc.parallelize(input);
      val modRDD = inputRDD.map(x => x % 10);
      val halfRDD = modRDD.filter(x => x > 5);
      output = halfRDD.reduce((x, y) => x + y);
    }

    val end = System.nanoTime()
    val t = (end - start) / (20 * 1000000.0);

    println("RDD output (%f ms): %d".format(t, output))
  }
}
