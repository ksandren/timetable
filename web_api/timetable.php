<?php
	$dblocation = "127.0.0.1";
	$dbname = "timetable";
	$dbuser = "admin";
	$dbpasswd = "pass";
	$query = "SELECT `день`.`date` AS `число`, `день`.`day` AS `День недели`, LEFT( `время`.`value`, 1 ) AS `пара`, `дисциплина`.`value` AS `Предмет`, `группа`.`value` AS `Поток`, `аудитория`.`value` AS `аудитория`, `перподаватель`.`value` AS `имя` FROM `timetable`.`день` AS `день`, `timetable`.`пара` AS `пара`, `timetable`.`время` AS `время`, `timetable`.`дисциплина` AS `дисциплина`, `timetable`.`аудитория` AS `аудитория`, `timetable`.`перподаватель` AS `перподаватель`, `timetable`.`группа` AS `группа`, `timetable`.`факультет` AS `факультет`, `timetable`.`курс` AS `курс`, `timetable`.`форма обучения` AS `форма обучения` WHERE `день`.`date`>'2015-11-15' AND `день`.`id` = `пара`.`day` AND `время`.`id` = `пара`.`time` AND `дисциплина`.`id` = `пара`.`obj` AND `аудитория`.`id` = `пара`.`room` AND `перподаватель`.`id` = `пара`.`teacher` AND `группа`.`id` = `пара`.`group` AND `факультет`.`id` = `пара`.`fak` AND `курс`.`id` = `пара`.`kurs` AND `форма обучения`.`id` = `пара`.`form` AND `группа`.`value` = 'ПИБ-31.2 (Прикладная информатика)'";
	if (isset($_GET['v'])){
		switch($_GET['v'])
		{
		case '2':
			$query = "SELECT ".
					"DATE_FORMAT(`день`.`date`, '%d.%m') AS `date`, ".
					"`день`.`day`, MID(`время`.`value`, 4, 5) AS `time`, ".
					"`аудитория`.`value` AS `room`, ".
					"@type := REVERSE(MID(@r := REVERSE(`дисциплина`.`value`), 2, LOCATE('(', @r) - 2)) AS `type`, ".
					"IF(@type != '', LEFT(`дисциплина`.`value`, CHAR_LENGTH(`дисциплина`.`value`) - CHAR_LENGTH(@type) - 3), `дисциплина`.`value`) AS `obj`, ".
					"`перподаватель`.`value` AS `teacher`, ".
					"`группа`.`value` ".
					"FROM ".
					"`timetable`.`перподаватель`AS `перподаватель`, ".
					"`timetable`.`пара` AS `пара`, ".
					"`timetable`.`дисциплина`AS `дисциплина`, ".
					"`timetable`.`день` AS `день`, ".
					"`timetable`.`группа` AS `группа`, ".
					"`timetable`.`время` AS `время`, ".
					"`timetable`.`аудитория` AS `аудитория` ".
					"WHERE ".
					"`перподаватель`.`id` = `пара`.`teacher` ".
					"AND `дисциплина`.`id` = `пара`.`obj` ".
					"AND `день`.`id` = `пара`.`day` ".
					"AND `группа`.`id` = `пара`.`group` ".
					"AND `время`.`id` = `пара`.`time` ".
					"AND `аудитория`.`id` = `пара`.`room` ".
					"AND `группа`.`value` = 'ПИБ-31.2 (Прикладная информатика)' ".
					"ORDER BY `день`.`date`, `время`.`value`";
					break;
		case '3':
			include 'tt_web_api_v3/tt_web_api_v3.php';
			exit;
		}
			
	}
	$dbcnx = @mysql_connect($dblocation, $dbuser, $dbpasswd);
	if (!mysql_set_charset('utf8', $dbcnx))
	{
		echo "Error: Unable to set the character set.\n";
		exit;
	}
	if (!$dbcnx)
	{
		echo "Error: 1.\n";
		exit;
	}
	if (!@mysql_select_db($dbname,$dbcnx) )
	{
		echo "Error: 2.\n";
		exit;
	}
	$res = mysql_query($query);
	if(!$res)
	{
		echo "Error: 3.\n";
		exit;
	}
	else
	{
		$rows = array();
		while($data = mysql_fetch_array($res))
		{ 
			$rows[] = $data;
		}
		echo json_encode($rows);
	}
	mysql_close();
?>