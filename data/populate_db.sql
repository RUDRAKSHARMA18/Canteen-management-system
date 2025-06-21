-- Populate menu_items table 

INSERT INTO menu_items (item_id, name, price, available) VALUES
(1, 'Paneer Tikka', 150.00, 1),
(2, 'Veggie Burger', 80.00, 1),
(3, 'Masala Dosa', 60.00, 1),
(4, 'Aloo Paratha', 50.00, 1),
(5, 'Chole Bhature', 90.00, 1),
(6, 'Veg Pulao', 100.00, 1),
(7, 'Palak Paneer', 140.00, 1),
(8, 'Idli Sambhar', 40.00, 1),
(9, 'Pav Bhaji', 70.00, 1),
(10, 'Veg Manchurian', 120.00, 1),
(11, 'Cheese Pizza', 200.00, 1),
(12, 'Veg Sandwich', 50.00, 1),
(13, 'Dal Makhani', 110.00, 1),
(14, 'Uttapam', 65.00, 1),
(15, 'Rajma Chawal', 85.00, 1),
(16, 'Veg Hakka Noodles', 130.00, 1),
(17, 'Malai Kofta', 160.00, 1),
(18, 'Samosa', 20.00, 1),
(19, 'Dhokla', 30.00, 1),
(20, 'Veg Spring Roll', 60.00, 1);

-- Populate inventory table
INSERT INTO inventory (item_id, quantity, low_stock_threshold) VALUES
(1, 100, 10),
(2, 150, 20),
(3, 200, 30),
(4, 120, 15),
(5, 80, 10),
(6, 90, 15),
(7, 70, 10),
(8, 250, 50),
(9, 110, 20),
(10, 60, 10),
(11, 40, 5),
(12, 180, 25),
(13, 95, 15),
(14, 130, 20),
(15, 100, 15),
(16, 75, 10),
(17, 50, 5),
(18, 300, 50),
(19, 200, 30),
(20, 140, 20);

-- Populate wallets table (phone number-based customer IDs)
INSERT INTO wallets (user_id, balance) VALUES
('9999111123', 1200.00),
('9999111124', 300.00),
('9999111125', 600.00),
('8888111123', 800.00),
('8888111124', 400.00),
('8888111125', 150.00),
('7777111123', 1000.00),
('7777111124', 500.00);

-- Populate discounts table
INSERT INTO discounts (discount_id, name, type, value, start_time, end_time, combo_items) VALUES
(1, 'Monsoon 10% Off', 'percentage', 10.0, strftime('%s', '2025-05-01 00:00:00'), strftime('%s', '2025-05-31 23:59:59'), ''),
(2, 'Flat 50 Off', 'fixed', 50.0, strftime('%s', '2025-05-10 00:00:00'), strftime('%s', '2025-05-20 23:59:59'), ''),
(3, 'Combo Deal', 'combo', 100.0, strftime('%s', '2025-05-15 00:00:00'), strftime('%s', '2025-05-25 23:59:59'), '1,9'),
(4, 'Weekend 15% Off', 'percentage', 15.0, strftime('%s', '2025-05-24 00:00:00'), strftime('%s', '2025-05-25 23:59:59'), ''),
(5, 'Expired Deal', 'fixed', 30.0, strftime('%s', '2025-04-01 00:00:00'), strftime('%s', '2025-04-30 23:59:59'), '');

-- Populate loyalty_points table (phone number-based customer IDs, no guests)
INSERT INTO loyalty_points (user_id, points) VALUES
('9999111123', 150),
('9999111124', 30),
('9999111125', 60),
('8888111123', 80),
('8888111124', 40),
('8888111125', 15),
('7777111123', 100),
('7777111124', 50);

-- Populate loyalty_transactions table (phone number-based customer IDs)
INSERT INTO loyalty_transactions (user_id, points, type, timestamp) VALUES
('9999111123', 100, 'earned', strftime('%s', '2025-05-01 10:00:00')),
('9999111123', -50, 'redeemed', strftime('%s', '2025-05-02 12:00:00')),
('9999111124', 30, 'earned', strftime('%s', '2025-05-03 14:00:00')),
('9999111125', 60, 'earned', strftime('%s', '2025-05-04 16:00:00')),
('8888111123', 80, 'earned', strftime('%s', '2025-05-05 18:00:00')),
('8888111124', 40, 'earned', strftime('%s', '2025-05-06 09:00:00')),
('8888111125', 15, 'earned', strftime('%s', '2025-05-07 11:00:00')),
('7777111123', 70, 'earned', strftime('%s', '2025-05-08 13:00:00')),
('7777111124', 50, 'earned', strftime('%s', '2025-05-09 15:00:00')),
('9999111123', 80, 'earned', strftime('%s', '2025-05-10 17:00:00')),
('9999111124', 15, 'earned', strftime('%s', '2025-05-11 19:00:00')),
('9999111125', 25, 'earned', strftime('%s', '2025-05-12 21:00:00')),
('8888111123', 50, 'earned', strftime('%s', '2025-05-13 10:00:00')),
('8888111124', 20, 'earned', strftime('%s', '2025-05-14 12:00:00')),
('8888111125', 10, 'earned', strftime('%s', '2025-05-15 14:00:00')),
('7777111123', 30, 'earned', strftime('%s', '2025-05-16 16:00:00'));

-- Populate orders table (50 orders: 25 completed, 10 pending, 10 canceled, 5 guest)
INSERT INTO orders (order_id, user_id, status, total, created_at) VALUES
-- Completed orders (phone number customers)
(1, '9999111123', 'completed', 300.00, strftime('%s', '2025-05-01 10:00:00')),
(2, '9999111124', 'completed', 150.00, strftime('%s', '2025-05-01 11:00:00')),
(3, '9999111125', 'completed', 400.00, strftime('%s', '2025-05-01 12:00:00')),
(4, '8888111123', 'completed', 200.00, strftime('%s', '2025-05-02 13:00:00')),
(5, '8888111124', 'completed', 500.00, strftime('%s', '2025-05-02 14:00:00')),
(6, '8888111125', 'completed', 100.00, strftime('%s', '2025-05-02 15:00:00')),
(7, '7777111123', 'completed', 250.00, strftime('%s', '2025-05-03 16:00:00')),
(8, '7777111124', 'completed', 350.00, strftime('%s', '2025-05-03 17:00:00')),
(9, '9999111123', 'completed', 180.00, strftime('%s', '2025-05-03 18:00:00')),
(10, '9999111124', 'completed', 120.00, strftime('%s', '2025-05-04 19:00:00')),
(11, '9999111125', 'completed', 280.00, strftime('%s', '2025-05-04 20:00:00')),
(12, '8888111123', 'completed', 170.00, strftime('%s', '2025-05-05 09:00:00')),
(13, '8888111124', 'completed', 420.00, strftime('%s', '2025-05-05 10:00:00')),
(14, '8888111125', 'completed', 190.00, strftime('%s', '2025-05-05 11:00:00')),
(15, '7777111123', 'completed', 600.00, strftime('%s', '2025-05-06 12:00:00')),
(16, '7777111124', 'completed', 130.00, strftime('%s', '2025-05-06 13:00:00')),
(17, '9999111123', 'completed', 270.00, strftime('%s', '2025-05-06 14:00:00')),
(18, '9999111124', 'completed', 380.00, strftime('%s', '2025-05-07 15:00:00')),
(19, '9999111125', 'completed', 160.00, strftime('%s', '2025-05-07 16:00:00')),
(20, '8888111123', 'completed', 140.00, strftime('%s', '2025-05-07 17:00:00')),
-- Completed guest orders
(21, NULL, 'completed', 320.00, strftime('%s', '2025-05-08 18:00:00')),
(22, NULL, 'completed', 200.00, strftime('%s', '2025-05-08 19:00:00')),
(23, NULL, 'completed', 450.00, strftime('%s', '2025-05-09 20:00:00')),
(24, NULL, 'completed', 220.00, strftime('%s', '2025-05-09 21:00:00')),
(25, NULL, 'completed', 550.00, strftime('%s', '2025-05-10 09:00:00')),
-- Pending orders
(26, '9999111123', 'pending', 310.00, strftime('%s', '2025-05-12 15:00:00')),
(27, '9999111124', 'pending', 165.00, strftime('%s', '2025-05-12 16:00:00')),
(28, '9999111125', 'pending', 430.00, strftime('%s', '2025-05-13 17:00:00')),
(29, '8888111123', 'pending', 210.00, strftime('%s', '2025-05-13 18:00:00')),
(30, '8888111124', 'pending', 520.00, strftime('%s', '2025-05-14 19:00:00')),
(31, '8888111125', 'pending', 125.00, strftime('%s', '2025-05-14 20:00:00')),
(32, '7777111123', 'pending', 280.00, strftime('%s', '2025-05-15 21:00:00')),
(33, '7777111124', 'pending', 360.00, strftime('%s', '2025-05-15 22:00:00')),
(34, '9999111123', 'pending', 185.00, strftime('%s', '2025-05-16 09:00:00')),
(35, '9999111124', 'pending', 145.00, strftime('%s', '2025-05-16 10:00:00')),
-- Canceled orders
(36, '9999111123', 'canceled', 290.00, strftime('%s', '2025-05-17 11:00:00')),
(37, '9999111124', 'canceled', 155.00, strftime('%s', '2025-05-17 12:00:00')),
(38, '9999111125', 'canceled', 410.00, strftime('%s', '2025-05-18 13:00:00')),
(39, '8888111123', 'canceled', 195.00, strftime('%s', '2025-05-18 14:00:00')),
(40, '8888111124', 'canceled', 510.00, strftime('%s', '2025-05-19 15:00:00')),
(41, '8888111125', 'canceled', 115.00, strftime('%s', '2025-05-19 16:00:00')),
(42, '7777111123', 'canceled', 265.00, strftime('%s', '2025-05-20 17:00:00')),
(43, '7777111124', 'canceled', 375.00, strftime('%s', '2025-05-20 18:00:00')),
(44, '9999111123', 'canceled', 170.00, strftime('%s', '2025-05-21 19:00:00')),
(45, '9999111124', 'canceled', 135.00, strftime('%s', '2025-05-21 20:00:00')),
-- Guest orders (pending and canceled)
(46, NULL, 'pending', 260.00, strftime('%s', '2025-05-22 09:00:00')),
(47, NULL, 'pending', 370.00, strftime('%s', '2025-05-22 10:00:00')),
(48, NULL, 'canceled', 175.00, strftime('%s', '2025-05-23 11:00:00')),
(49, NULL, 'canceled', 155.00, strftime('%s', '2025-05-23 12:00:00')),
(50, NULL, 'pending', 340.00, strftime('%s', '2025-05-24 13:00:00'));

-- Populate order_items table (~100 items across orders)
INSERT INTO order_items (order_id, item_id, quantity, price) VALUES
-- Order 1
(1, 1, 1, 150.00),
(1, 9, 2, 70.00),
-- Order 2
(2, 2, 1, 80.00),
(2, 18, 2, 20.00),
-- Order 3
(3, 11, 1, 200.00),
(3, 7, 1, 140.00),
-- Order 4
(4, 5, 1, 90.00),
(4, 12, 2, 50.00),
-- Order 5
(5, 17, 2, 160.00),
(5, 16, 1, 130.00),
-- Order 6
(6, 8, 2, 40.00),
(6, 19, 1, 30.00),
-- Order 7
(7, 6, 1, 100.00),
(7, 14, 2, 65.00),
-- Order 8
(8, 13, 2, 110.00),
(8, 10, 1, 120.00),
-- Order 9
(9, 3, 2, 60.00),
(9, 20, 1, 60.00),
-- Order 10
(10, 4, 2, 50.00),
(10, 18, 1, 20.00),
-- Order 11
(11, 15, 2, 85.00),
(11, 12, 1, 50.00),
-- Order 12
(12, 2, 1, 80.00),
(12, 19, 3, 30.00),
-- Order 13
(13, 11, 2, 200.00),
(13, 8, 1, 40.00),
-- Order 14
(14, 9, 2, 70.00),
(14, 18, 1, 20.00),
-- Order 15
(15, 1, 2, 150.00),
(15, 7, 1, 140.00),
-- Order 16
(16, 4, 2, 50.00),
(16, 12, 1, 50.00),
-- Order 17
(17, 6, 1, 100.00),
(17, 14, 2, 65.00),
-- Order 18
(18, 13, 2, 110.00),
(18, 10, 1, 120.00),
-- Order 19
(19, 2, 1, 80.00),
(19, 18, 2, 20.00),
-- Order 20
(20, 8, 2, 40.00),
(20, 19, 1, 30.00),
-- Order 21 (guest)
(21, 11, 1, 200.00),
(21, 9, 2, 70.00),
-- Order 22 (guest)
(22, 3, 2, 60.00),
(22, 12, 1, 50.00),
-- Order 23 (guest)
(23, 17, 2, 160.00),
(23, 14, 1, 65.00),
-- Order 24 (guest)
(24, 5, 1, 90.00),
(24, 18, 3, 20.00),
-- Order 25 (guest)
(25, 1, 2, 150.00),
(25, 7, 1, 140.00),
-- Order 26
(26, 15, 2, 85.00),
(26, 12, 1, 50.00),
-- Order 27
(27, 2, 1, 80.00),
(27, 19, 3, 30.00),
-- Order 28
(28, 11, 2, 200.00),
(28, 8, 1, 40.00),
-- Order 29
(29, 9, 2, 70.00),
(29, 18, 1, 20.00),
-- Order 30
(30, 17, 2, 160.00),
(30, 16, 1, 130.00),
-- Order 31
(31, 8, 2, 40.00),
(31, 19, 1, 30.00),
-- Order 32
(32, 6, 1, 100.00),
(32, 14, 2, 65.00),
-- Order 33
(33, 13, 2, 110.00),
(33, 10, 1, 120.00),
-- Order 34
(34, 3, 2, 60.00),
(34, 20, 1, 60.00),
-- Order 35
(35, 4, 2, 50.00),
(35, 18, 1, 20.00),
-- Order 36
(36, 15, 2, 85.00),
(36, 12, 1, 50.00),
-- Order 37
(37, 2, 1, 80.00),
(37, 19, 3, 30.00),
-- Order 38
(38, 11, 2, 200.00),
(38, 8, 1, 40.00),
-- Order 39
(39, 9, 2, 70.00),
(39, 18, 1, 20.00),
-- Order 40
(40, 17, 2, 160.00),
(40, 16, 1, 130.00),
-- Order 41
(41, 8, 2, 40.00),
(41, 19, 1, 30.00),
-- Order 42
(42, 6, 1, 100.00),
(42, 14, 2, 65.00),
-- Order 43
(43, 13, 2, 110.00),
(43, 10, 1, 120.00),
-- Order 44
(44, 3, 2, 60.00),
(44, 20, 1, 60.00),
-- Order 45
(45, 4, 2, 50.00),
(45, 18, 1, 20.00),
-- Order 46 (guest)
(46, 15, 2, 85.00),
(46, 12, 1, 50.00),
-- Order 47 (guest)
(47, 11, 1, 200.00),
(47, 9, 2, 70.00),
-- Order 48 (guest)
(48, 3, 2, 60.00),
(48, 12, 1, 50.00),
-- Order 49 (guest)
(49, 5, 1, 90.00),
(49, 18, 3, 20.00),
-- Order 50 (guest)
(50, 17, 2, 160.00),
(50, 14, 1, 65.00);

-- Populate bills table (25 bills for completed orders, including guests)
INSERT INTO bills (bill_id, order_id, tax, total, payment_method, created_at, refunded) VALUES
(1, 1, 24.00, 324.00, 'Wallet', strftime('%s', '2025-05-01 10:30:00'), 0),
(2, 2, 12.00, 162.00, 'Wallet', strftime('%s', '2025-05-01 11:30:00'), 0),
(3, 3, 32.00, 432.00, 'Wallet', strftime('%s', '2025-05-01 12:30:00'), 0),
(4, 4, 16.00, 216.00, 'Wallet', strftime('%s', '2025-05-02 13:30:00'), 0),
(5, 5, 40.00, 540.00, 'Wallet', strftime('%s', '2025-05-02 14:30:00'), 0),
(6, 6, 8.00, 108.00, 'Wallet', strftime('%s', '2025-05-02 15:30:00'), 0),
(7, 7, 20.00, 270.00, 'Wallet', strftime('%s', '2025-05-03 16:30:00'), 0),
(8, 8, 28.00, 378.00, 'Wallet', strftime('%s', '2025-05-03 17:30:00'), 0),
(9, 9, 14.40, 194.40, 'Wallet', strftime('%s', '2025-05-03 18:30:00'), 0),
(10, 10, 9.60, 129.60, 'Wallet', strftime('%s', '2025-05-04 19:30:00'), 0),
(11, 11, 22.40, 302.40, 'Wallet', strftime('%s', '2025-05-04 20:30:00'), 0),
(12, 12, 13.60, 183.60, 'Wallet', strftime('%s', '2025-05-05 09:30:00'), 0),
(13, 13, 33.60, 453.60, 'Wallet', strftime('%s', '2025-05-05 10:30:00'), 0),
(14, 14, 15.20, 205.20, 'Wallet', strftime('%s', '2025-05-05 11:30:00'), 0),
(15, 15, 48.00, 648.00, 'Wallet', strftime('%s', '2025-05-06 12:30:00'), 0),
(16, 16, 10.40, 140.40, 'Wallet', strftime('%s', '2025-05-06 13:30:00'), 0),
(17, 17, 21.60, 291.60, 'Wallet', strftime('%s', '2025-05-06 14:30:00'), 0),
(18, 18, 30.40, 410.40, 'Wallet', strftime('%s', '2025-05-07 15:30:00'), 0),
(19, 19, 12.80, 172.80, 'Wallet', strftime('%s', '2025-05-07 16:30:00'), 0),
(20, 20, 11.20, 151.20, 'Wallet', strftime('%s', '2025-05-07 17:30:00'), 0),
-- Guest bills (Cash or Card)
(21, 21, 25.60, 345.60, 'Cash', strftime('%s', '2025-05-08 18:30:00'), 0),
(22, 22, 16.00, 216.00, 'Card', strftime('%s', '2025-05-08 19:30:00'), 0),
(23, 23, 36.00, 486.00, 'Cash', strftime('%s', '2025-05-09 20:30:00'), 0),
(24, 24, 17.60, 237.60, 'Card', strftime('%s', '2025-05-09 21:30:00'), 0),
(25, 25, 44.00, 594.00, 'Cash', strftime('%s', '2025-05-10 09:30:00'), 0);

-- Populate activity_log table (~100 logs)
INSERT INTO activity_log (user_id, action, timestamp) VALUES
('9999111123', 'Order created: order_id 1', strftime('%s', '2025-05-01 10:00:00')),
('9999111123', 'Bill generated: order_id 1', strftime('%s', '2025-05-01 10:30:00')),
('9999111124', 'Order created: order_id 2', strftime('%s', '2025-05-01 11:00:00')),
('9999111124', 'Bill generated: order_id 2', strftime('%s', '2025-05-01 11:30:00')),
('9999111125', 'Order created: order_id 3', strftime('%s', '2025-05-01 12:00:00')),
('9999111125', 'Bill generated: order_id 3', strftime('%s', '2025-05-01 12:30:00')),
('8888111123', 'Order created: order_id 4', strftime('%s', '2025-05-02 13:00:00')),
('8888111123', 'Bill generated: order_id 4', strftime('%s', '2025-05-02 13:30:00')),
('8888111124', 'Order created: order_id 5', strftime('%s', '2025-05-02 14:00:00')),
('8888111124', 'Bill generated: order_id 5', strftime('%s', '2025-05-02 14:30:00')),
('8888111125', 'Order created: order_id 6', strftime('%s', '2025-05-02 15:00:00')),
('8888111125', 'Bill generated: order_id 6', strftime('%s', '2025-05-02 15:30:00')),
('7777111123', 'Order created: order_id 7', strftime('%s', '2025-05-03 16:00:00')),
('7777111123', 'Bill generated: order_id 7', strftime('%s', '2025-05-03 16:30:00')),
('7777111124', 'Order created: order_id 8', strftime('%s', '2025-05-03 17:00:00')),
('7777111124', 'Bill generated: order_id 8', strftime('%s', '2025-05-03 17:30:00')),
('9999111123', 'Order created: order_id 9', strftime('%s', '2025-05-03 18:00:00')),
('9999111123', 'Bill generated: order_id 9', strftime('%s', '2025-05-03 18:30:00')),
('9999111124', 'Order created: order_id 10', strftime('%s', '2025-05-04 19:00:00')),
('9999111124', 'Bill generated: order_id 10', strftime('%s', '2025-05-04 19:30:00')),
('9999111125', 'Order created: order_id 11', strftime('%s', '2025-05-04 20:00:00')),
('9999111125', 'Bill generated: order_id 11', strftime('%s', '2025-05-04 20:30:00')),
('8888111123', 'Order created: order_id 12', strftime('%s', '2025-05-05 09:00:00')),
('8888111123', 'Bill generated: order_id 12', strftime('%s', '2025-05-05 09:30:00')),
('8888111124', 'Order created: order_id 13', strftime('%s', '2025-05-05 10:00:00')),
('8888111124', 'Bill generated: order_id 13', strftime('%s', '2025-05-05 10:30:00')),
('8888111125', 'Order created: order_id 14', strftime('%s', '2025-05-05 11:00:00')),
('8888111125', 'Bill generated: order_id 14', strftime('%s', '2025-05-05 11:30:00')),
('7777111123', 'Order created: order_id 15', strftime('%s', '2025-05-06 12:00:00')),
('7777111123', 'Bill generated: order_id 15', strftime('%s', '2025-05-06 12:30:00')),
('7777111124', 'Order created: order_id 16', strftime('%s', '2025-05-06 13:00:00')),
('7777111124', 'Bill generated: order_id 16', strftime('%s', '2025-05-06 13:30:00')),
('9999111123', 'Order created: order_id 17', strftime('%s', '2025-05-06 14:00:00')),
('9999111123', 'Bill generated: order_id 17', strftime('%s', '2025-05-06 14:30:00')),
('9999111124', 'Order created: order_id 18', strftime('%s', '2025-05-07 15:00:00')),
('9999111124', 'Bill generated: order_id 18', strftime('%s', '2025-05-07 15:30:00')),
('9999111125', 'Order created: order_id 19', strftime('%s', '2025-05-07 16:00:00')),
('9999111125', 'Bill generated: order_id 19', strftime('%s', '2025-05-07 16:30:00')),
('8888111123', 'Order created: order_id 20', strftime('%s', '2025-05-07 17:00:00')),
('8888111123', 'Bill generated: order_id 20', strftime('%s', '2025-05-07 17:30:00')),
(NULL, 'Order created: order_id 21', strftime('%s', '2025-05-08 18:00:00')),
(NULL, 'Bill generated: order_id 21', strftime('%s', '2025-05-08 18:30:00')),
(NULL, 'Order created: order_id 22', strftime('%s', '2025-05-08 19:00:00')),
(NULL, 'Bill generated: order_id 22', strftime('%s', '2025-05-08 19:30:00')),
(NULL, 'Order created: order_id 23', strftime('%s', '2025-05-09 20:00:00')),
(NULL, 'Bill generated: order_id 23', strftime('%s', '2025-05-09 20:30:00')),
(NULL, 'Order created: order_id 24', strftime('%s', '2025-05-09 21:00:00')),
(NULL, 'Bill generated: order_id 24', strftime('%s', '2025-05-09 21:30:00')),
(NULL, 'Order created: order_id 25', strftime('%s', '2025-05-10 09:00:00')),
(NULL, 'Bill generated: order_id 25', strftime('%s', '2025-05-10 09:30:00')),
('9999111123', 'Order created: order_id 26', strftime('%s', '2025-05-12 15:00:00')),
('9999111124', 'Order created: order_id 27', strftime('%s', '2025-05-12 16:00:00')),
('9999111125', 'Order created: order_id 28', strftime('%s', '2025-05-13 17:00:00')),
('8888111123', 'Order created: order_id 29', strftime('%s', '2025-05-13 18:00:00')),
('8888111124', 'Order created: order_id 30', strftime('%s', '2025-05-14 19:00:00')),
('8888111125', 'Order created: order_id 31', strftime('%s', '2025-05-14 20:00:00')),
('7777111123', 'Order created: order_id 32', strftime('%s', '2025-05-15 21:00:00')),
('7777111124', 'Order created: order_id 33', strftime('%s', '2025-05-15 22:00:00')),
('9999111123', 'Order created: order_id 34', strftime('%s', '2025-05-16 09:00:00')),
('9999111124', 'Order created: order_id 35', strftime('%s', '2025-05-16 10:00:00')),
('9999111123', 'Order created: order_id 36', strftime('%s', '2025-05-17 11:00:00')),
(NULL, 'Order canceled: order_id 36', strftime('%s', '2025-05-17 11:30:00')),
('9999111124', 'Order created: order_id 37', strftime('%s', '2025-05-17 12:00:00')),
(NULL, 'Order canceled: order_id 37', strftime('%s', '2025-05-17 12:30:00')),
('9999111125', 'Order created: order_id 38', strftime('%s', '2025-05-18 13:00:00')),
(NULL, 'Order canceled: order_id 38', strftime('%s', '2025-05-18 13:30:00')),
('8888111123', 'Order created: order_id 39', strftime('%s', '2025-05-18 14:00:00')),
(NULL, 'Order canceled: order_id 39', strftime('%s', '2025-05-18 14:30:00')),
('8888111124', 'Order created: order_id 40', strftime('%s', '2025-05-19 15:00:00')),
(NULL, 'Order canceled: order_id 40', strftime('%s', '2025-05-19 15:30:00')),
('8888111125', 'Order created: order_id 41', strftime('%s', '2025-05-19 16:00:00')),
(NULL, 'Order canceled: order_id 41', strftime('%s', '2025-05-19 16:30:00')),
('7777111123', 'Order created: order_id 42', strftime('%s', '2025-05-20 17:00:00')),
(NULL, 'Order canceled: order_id 42', strftime('%s', '2025-05-20 17:30:00')),
('7777111124', 'Order created: order_id 43', strftime('%s', '2025-05-20 18:00:00')),
(NULL, 'Order canceled: order_id 43', strftime('%s', '2025-05-20 18:30:00')),
('9999111123', 'Order created: order_id 44', strftime('%s', '2025-05-21 19:00:00')),
(NULL, 'Order canceled: order_id 44', strftime('%s', '2025-05-21 19:30:00')),
('9999111124', 'Order created: order_id 45', strftime('%s', '2025-05-21 20:00:00')),
(NULL, 'Order canceled: order_id 45', strftime('%s', '2025-05-21 20:30:00')),
(NULL, 'Order created: order_id 46', strftime('%s', '2025-05-22 09:00:00')),
(NULL, 'Order created: order_id 47', strftime('%s', '2025-05-22 10:00:00')),
(NULL, 'Order created: order_id 48', strftime('%s', '2025-05-23 11:00:00')),
(NULL, 'Order canceled: order_id 48', strftime('%s', '2025-05-23 11:30:00')),
(NULL, 'Order created: order_id 49', strftime('%s', '2025-05-23 12:00:00')),
(NULL, 'Order canceled: order_id 49', strftime('%s', '2025-05-23 12:30:00')),
(NULL, 'Order created: order_id 50', strftime('%s', '2025-05-24 13:00:00'));

-- Populate settings table
INSERT INTO settings (key, value) VALUES
('tax_rate', 0.08),
('loyalty_earn_rate', 10.0);
