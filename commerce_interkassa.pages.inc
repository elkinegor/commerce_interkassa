<?php
/**
 * @file
 * Status page.
 */

/**
 * Page callback for payment method commerce_interkassa.
 *
 * @param int $order
 *   Commerce order.
 *
 * @return bool
 *   Check returned data.
 */
function commerce_interkassa_payment_status($order) {
  $request_data = $_POST;
  commerce_interkassa_add_log('Order number @order_number request data: @request_data', array(
    '@order_number' => $order->order_id,
    '@request_data' => print_r($request_data, 1),
  ));
	
	$settings = commerce_payment_method_instance_load('commerce_interkassa|commerce_payment_commerce_interkassa');
			
/*
  if ($request_data['ik_inv_st'] == 'success') {
    if ($order) {
      $order_wrapper = entity_metadata_wrapper('commerce_order', $order);
      $amount        = $order_wrapper->commerce_order_total->amount->value();
      $currency_code = $order_wrapper->commerce_order_total->currency_code->value();
      $settings      = commerce_payment_method_instance_load('commerce_interkassa|commerce_payment_commerce_interkassa');
			drupal_set_message(print_r());

      $sing_hash_str = $settings['settings']['shop_id'] . ':' .
        commerce_currency_amount_to_decimal($amount, $currency_code) . ':' .
        $order->order_id . ':' .
        $request_data['ik_paysystem_alias'] . ':' .
        $request_data['ik_baggage_fields'] . ':' .
        $request_data['ik_payment_state'] . ':' .
        $request_data['ik_trans_id'] . ':' .
        $request_data['ik_currency_exch'] . ':' .
        $request_data['ik_fees_payer'] . ':' .
        $settings['settings']['secret_key'];

      $sign_hash = strtoupper(md5($sing_hash_str));
			
			
      if ($request_data['ik_sign'] === $sign_hash) {
        if ($order->status !== 'completed') {
          commerce_interkassa_create_transaction($order);
          return t('Payment the order number @order_number is complited', array('@order_number' => $order->order_id));
        }
      }
      else {
        drupal_set_message(t('If the error persists then, contact your site administrator.'), 'warning');
        return t('Payment the order number @order_number is fail', array('@order_number' => $order->order_id));
      }
    }
  }*/ 
	unset($request_data['ik_sign']);
	ksort($request_data, SORT_STRING); // сортируем по ключам в алфавитном порядке элементы массива  
	array_push($request_data, $settings['settings']['secret_key']); 
	// добавляем в конец массива "секретный ключ" 
	$signString = implode(':', $request_data); // конкатенируем значения через символ ":" 
	
	commerce_interkassa_add_log('Данные для генерации секретного ключа @data @data2 @data3', array(
		'@data' => print_r($request_data, true),
		'@data2' => print_r($signString, true),
		'@data3' => md5($signString, true),
	));
	
	$sign = base64_encode(md5($signString, true));  // берем MD5 хэш в бинарном виде по сформированной строке и кодируем в BASE64

	
	if ($_POST['ik_sign'] === $sign) {

		switch ($request_data['ik_inv_st']) {
        case "fail" :
					$transaction = commerce_payment_transaction_new('commerce_interkassa', $request_data['ik_pm_no']);
					$transaction->instance_id = 'commerce_interkassa|commerce_payment_commerce_interkassa';
					$transaction->amount = $request_data['ik_am']*100;
					$transaction->remote_id = $request_data['ik_inv_id'];
					$transaction->currency_code = 'RUB';
					$transaction->status = COMMERCE_PAYMENT_STATUS_FAILURE;
					$transaction->message = t('Не прошел');
					commerce_payment_transaction_save($transaction);
					echo "200 OK";	
		 		break;
        case "success" :
					$transaction = commerce_payment_transaction_new('commerce_interkassa', $request_data['ik_pm_no']);
					$transaction->instance_id = 'commerce_interkassa|commerce_payment_commerce_interkassa';
					$transaction->amount = $request_data['ik_am']*100;
					$transaction->remote_id = $request_data['ik_inv_id'];
					$transaction->currency_code = 'RUB';
					$transaction->status = COMMERCE_PAYMENT_STATUS_SUCCESS;
					$transaction->message = t('Оплачено');
					commerce_payment_transaction_save($transaction);
					echo "200 OK";
		  	break;
      }
		
	} else {
		
		commerce_interkassa_add_log('Ошибка ik_sign @ik_sign_post != @ik_sign', array(
			'@ik_sign_post' => $_POST['ik_sign'],
			'@ik_sign' => $sign,
		));
		return FALSE;
	}

}
