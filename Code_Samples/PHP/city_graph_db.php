<?php

assert(!empty($_GET['city']));
assert(!empty($_GET['cat']));
assert(!empty($_GET['date1']));

$city = $_GET['city'];
$cat = $_GET['cat'];
$tableName = $city.'_'.$cat;
/*
print $city;
print '<br />';
print $cat;
*/

$date1 = GetFormatedDate($_GET['date1']);
$date2 = "";
if (empty($_GET['date2']))
{
  // $date2 = "12-Mar-2007";
  $date2 = date("d-M-y");
}
else
  $date2 = GetFormatedDate($_GET['date2']);
// assert mktime(date1) < mktime(date2);

$search = "";
if (!empty($_GET['search']))
  $search = $_GET['search'];

$dates = array();
$postingCounts = array();
$unixDates = array();


{
  $conn = oci_connect('craigtest', 'password', 'username');
  if (!$conn) {
    $e = oci_error();
    print htmlentities($e['message']);
    exit;
  }

  if (empty($search))
    $query = "select posted, count(posted), (posted - to_date('01-jan-1970','dd-mon-yyyy'))*(24*60*60) from ".$tableName." where posted between '".$date1."' and '".$date2."' group by posted order by posted";
  else
    $query = "select posted, count(posted), (posted - to_date('01-jan-1970','dd-mon-yyyy'))*(24*60*60) from ".$tableName." where posted between '".$date1."' and '".$date2."' and lower(html) like lower('%".$search."%') group by posted order by posted";  
  // $query = "select posted, count(posted), (posted - to_date('01-jan-1970','dd-mon-yyyy'))*(24*60*60) from ".$tableName." where posted >= '".$date1."' group by posted order by posted";
  // $query = "select posted, count(posted), (posted - to_date('01-jan-1970','dd-mon-yyyy'))*(24*60*60) from ".$tableName." group by posted order by posted";

  // print $query; exit;

  $stid = oci_parse($conn, $query);
  if (!$stid) {
    $e = oci_error($conn);
    print htmlentities($e['message']);
    exit;
  }

  $r = oci_execute($stid, OCI_DEFAULT);
  if (!$r) {
    $e = oci_error($stid);
    echo htmlentities($e['message']);
    exit;
  }

  while ($row = oci_fetch_array($stid, OCI_RETURN_NULLS)) {
    $dates[] = $row[0];
    $postingCounts[] = $row[1];
    $unixDates[] = $row[2];
  }
  assert((count($dates) == count($postingCounts)) && (count($postingCounts) == count($unixDates)));
  # print_r($cities);

  oci_close($conn);
/*
  print '<table border="1">';
  for ($i = 0; $i < count($dates); ++$i) {
    print '<tr>';
    print '<td>'.$dates[$i].'</td>';
    print '<td>'.$unixDates[$i].'</td>';
    print '<td>'.$postingCounts[$i].'</td>';
    print '</tr>';
  }
  print '</table>';
*/
}

include ("JpGraph/jpgraph-2.1.4/src/jpgraph.php");
include ("JpGraph/jpgraph-2.1.4/src/jpgraph_line.php");
include ("JpGraph/jpgraph-2.1.4/src/jpgraph_utils.inc.php");

// Now get labels at the start of each month
$dateScaleUtils = new DateScaleUtils();
list($tickPositions,$minTickPositions) = $dateScaleUtils->GetTicks($unixDates, DSUTILS_DAY1);
//list($tickPositions,$minTickPositions) = $dateScaleUtils->GetTicks($unixDates, DSUTILS_WEEK1);
//list($tickPositions,$minTickPositions) = $dateScaleUtils->GetTicks($unixDates, DSUTILS_MONTH1);

// We add some grace to the end of the X-axis scale so that the first and last
// data point isn't exactly at the very end or beginning of the scale
$grace = 20000; // original 400000;
$xmin = $unixDates[0]-$grace;
$xmax = $unixDates[count($unixDates)-1]+$grace;

//
// The code to setup a very basic graph
//
$graph = new Graph(1000,600);

//
// We use an integer scale on the X-axis since the positions on the X axis
// are assumed to be UNI timestamps
$graph->SetScale('intlin', 0, 0, $xmin, $xmax);
$graph->title->Set('Days on X axis with manual ticks');
$graph->title->SetFont(FF_ARIAL,FS_NORMAL,12);

//
// Make sure that the X-axis is always at the bottom of the scale
// (By default the X-axis is always positioned at Y=0 so if the scale
// doesn't happen to include 0 the axis will not be shown)
$graph->xaxis->SetPos('min');

// Now set the tic positions
$graph->xaxis->SetTickPositions($tickPositions,$minTickPositions);

// The labels should be formatted at dates with "Year-month"
$graph->xaxis->SetLabelFormatString('M d y',true);
// Set the angle for the labels to 90 degrees
$graph->xaxis->SetLabelAngle(90);

$graph->SetShadow();
$graph->SetMargin(40,20,30,70);

// Use Ariel font
$graph->xaxis->SetFont(FF_ARIAL,FS_NORMAL,9);

// Add a X-grid
$graph->xgrid->Show();

// Create the plot line
$p1 = new LinePlot($postingCounts,$unixDates);
//$p1->SetColor('black');
$p1->SetWeight(2);
$graph->Add($p1);

// Output graph
$graph->Stroke();
?>

<?php
function MonthIntToStr($monthInt)
{
  $month = "";
  if ($monthInt == 1) $month = "Jan";
  else if ($monthInt == 2) $month = "Feb";
  else if ($monthInt == 3) $month = "Mar";
  else if ($monthInt == 4) $month = "Apr";
  else if ($monthInt == 5) $month = "May";
  else if ($monthInt == 6) $month = "Jun";
  else if ($monthInt == 7) $month = "Jul";
  else if ($monthInt == 8) $month = "Aug";
  else if ($monthInt == 9) $month = "Sep";
  else if ($monthInt == 10) $month = "Oct";
  else if ($monthInt == 11) $month = "Nov";
  else if ($monthInt == 12) $month = "Dec";
  else assert(false);
  return $month;
}

function GetFormatedDate($dateIn)
{
  $dateParts = explode("-", $dateIn);
  $day = sprintf("%02d", $dateParts[0]);
  $monthInt = (int) $dateParts[1];
  $year = $dateParts[2];
  // monthInt -> month
  $month = MonthIntToStr($monthInt);
  /*
  print $tableName;
  print '<br />';
  print $day.'***'.$month.'***'.$year.'<br />';
  */
  $date = $day."-".$month."-".$year;
  return $date;
}
?>
