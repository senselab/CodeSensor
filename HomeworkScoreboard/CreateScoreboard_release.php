
<?php

$use_base_user = false;
$baseM = 26736; 
$baseI = 38753228;

$powerI = 1;
$powerM = 1;
$priority_tone = 0.5;

$score_P_ratio = 0.3;
$TOTAL_P_SCORE = 80*60;


function calculate_priority()
{
	global $score_board;
	global $baseI;
	global $baseM;
	global $powerI ;
	global $powerM;
	global $priority_tone;
	global $use_base_user;
	
	if ( $use_base_user ) {

		for ($k=0; $k < count($score_board); ++$k) {
			if ($score_board[$k]["id"] == "1000001") {
				break;
			}
		}
	
		if ($k < count($score_board)) {
			
			if ( array_key_exists("I_refs", $score_board[$k]) && array_key_exists("mem_total",$score_board[$k])) {
				if ( $score_board[$k]["I_refs"] != -1 && $score_board[$k]["mem_total"] != -1) {
					$baseI = $score_board[$k]["I_refs"];
					$baseM = $score_board[$k]["mem_total"];
				}
			}
		}
	}
		
	for ( $k = 0; $k < count($score_board); ++$k) {
		if ( IsValidScoreboardEntry($k) && $score_board[$k]["check_pattern"] == 1 ) {
			
			$perf_I = $baseI / (1+$score_board[$k]["I_refs"]);
			$perf_M = $baseM / (1+$score_board[$k]["mem_total"]);
			$priority = pow($perf_I,$powerI) * pow($perf_M, $powerM);
			$priority = pow($priority, $priority_tone);
		//	echo $priority . "\n";

		}
		else {
			$priority = 0;
		}	
	
		$score_board[$k]["priority"] = $priority;
	}
	
}

function calculate_score()
{
	global $score_board;
	global $TOTAL_P_SCORE;
	global $score_P_ratio;
	
	$qCur = new SplPriorityQueue();
	$qExpire = new SplPriorityQueue();

	$total_p_score = $TOTAL_P_SCORE;
	$multiplier = 0.01;
	
	
	$qCur->setExtractFlags(SplPriorityQueue::EXTR_DATA);
	$qExpire->setExtractFlags(SplPriorityQueue::EXTR_DATA);
	
	foreach($score_board as $k=>$r) {
	
		
		///// f_score
		if (IsValidScoreboardEntry($k)) {
						
			$score_board[$k]["func"] = $score_board[$k]["check_pattern"]*pow(0.9,$score_board[$k]["mem_error"]);
						
			if ( $score_board[$k]["mem_heap"] >0) {
				$score_board[$k]["func"] *= ($score_board[$k]["mem_heap"] -$score_board[$k]["heap_lost"]) / ($score_board[$k]["mem_heap"]);
			}

			if ( $r["priority"]>0)
				$qCur->insert($k, $r["priority"]);
		}
		else {
			$score_board[$k]["func"] = 0;
		}
		
		////// init p_score
		$score_board[$k]["p_score"] = 0;
	}
	
	while ($qCur->valid() && $total_p_score >0) {
		while($qCur->valid() ) {
			$idx = $qCur->extract();
	
		
			$s = $score_board[$idx]["priority"]*$multiplier;	// alloted score in the scheduling cycle
			
			if ( $s > 100 - $score_board[$idx]["p_score"])
				$s =  100 - $score_board[$idx]["p_score"];
				
			$score_board[$idx]["p_score"] += $s;
			$total_p_score -= $s;
					
			if ($score_board[$idx]["p_score"] < 100) {
				$qExpire->insert($idx, $score_board[$idx]["priority"]);
			}

		}
		
		$qCur = $qExpire;
		$qExpire = new SplPriorityQueue();
		$qExpire->setExtractFlags(SplPriorityQueue::EXTR_DATA);	
	}
	
	foreach($score_board as $k=>$r) {
		$score_board[$k]["score"] = 100*$score_board[$k]["func"]*((1-$score_P_ratio) + $score_board[$k]["p_score"]*$score_P_ratio/100);
	}
	
	////////sort by score
	
	$tmp = Array();
	
	foreach($score_board as &$ma)
    	$tmp[] = &$ma["score"];
	array_multisort($tmp,SORT_DESC, $score_board ); 	
}


?>
